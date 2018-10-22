#include <iostream>
#include <iomanip>
#include <cmath>

#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/math/Math.h>

using namespace yarp::math;

#define _print std::cout << std::setw(20) << std::left

#define printLags          0
#define printAngles        1
#define printSpaceAng      1
#define printNormalAng     1
#define printTrueNormalAng 0
#define printMicAng        0
#define printAngleIndex    1
#define printEgoMapFront   1
#define printEgoMapLo      0


inline int myMod(int a, int b) {
	return  a >= 0 ? a % b : (a % b) + b;
}


void yarpPrintVector(const yarp::sig::Vector &outVector) {
	for (int i = 0; i < outVector.size(); i++) {
		if (i % 5 == 0) std::cout << std::endl;
			std::cout << std::setw(12) << outVector[i] << " ";    
		} std::cout << "\n\n";
}


yarp::sig::Vector yarpRollVector(yarp::sig::Vector &sourceVector, int roll_by) {

	yarp::sig::Vector targetVector(sourceVector.size());

	int roll_to     = sourceVector.size() - roll_by;
	int current_pos = roll_to;

	for (int i = 0; i < roll_by; i++) {
		targetVector[i] = sourceVector[current_pos++];
	}
	current_pos = roll_by;
	for (int i = 0; i < roll_to; i++) {
		targetVector[current_pos++] = sourceVector[i];
	}

	return targetVector;
}


yarp::sig::Vector yarpUnwrap(yarp::sig::Vector &sourceVector) {

	const double pi = 2 * acos(0.0);
	int len = sourceVector.size();

	yarp::sig::Vector targetVector(len);
	
	targetVector[0] = sourceVector[0];
	for (int i = 1; i < len; i++) {
		double d = sourceVector[i] - sourceVector[i-1];
		d = d > pi ? d - 2 * pi : (d < -pi ? d + 2 * pi : d);
		targetVector[i] = targetVector[i-1] + d;
	}
	
	return targetVector;
}


yarp::sig::Vector setLowResolutionMap(yarp::sig::Vector &sourceVector) {


	int nBeams = sourceVector.size();
	int nBeamsPerHemi = 21;

	yarp::sig::Vector targetVector(nBeams*2-2);

	int current_beam = 0;
	for (int beam = nBeams-nBeamsPerHemi-1; beam > 0; beam--) {
		targetVector[current_beam++] = sourceVector[beam];
	}

	for (int beam = 0; beam < nBeams; beam++) {
		targetVector[current_beam++] = sourceVector[beam];
	}

	for (int beam = nBeams-2; beam > nBeams-nBeamsPerHemi-1; beam--) {
		targetVector[current_beam++] = sourceVector[beam];
	}

	return targetVector;
}



double lininterp(double x, double x1, double y1, double x2, double y2) {
    return y1 + ((y2 - y1) * (x - x1)) / (x2 - x1);
}

inline double myRound(double a) { 
	return  a >= 0.000000001 ? ((a - (int)a >= 0.50) ? ((int)a + 1) : ((int)a)) : ((a - (int)a <= -0.50) ? ((int)a-1) : ((int)a)); 
}

void frontFieldMirror(yarp::sig::Vector &target, yarp::sig::Vector &source) {
	
	//-- Make sure space is allocated.
	target.resize(source.size() * 2, 0.0);

	//-- Get the length of the source vector.
	int full_length = source.size();
	int half_length = full_length / 2;

	//-- Use this for continuous iteration.
	int current_position = 0;
	
	//-- Mirror First Quarter with Second.
	for (int index = full_length - half_length - 1; index > -1; index--) {
		target[current_position++] = source[index];
	}
	
	//-- Set Second and Third Quarter as normal.
	for (int index = 0; index < full_length; index++) {
		target[current_position++] = source[index];
	}
	
	//-- Mirror Fourth Quarter with Third.
	for (int index = full_length - 1; index > full_length - half_length - 1; index--) {
		target[current_position++] = source[index];
	}

}

void frontFieldMirror_alt(yarp::sig::Vector &target, yarp::sig::Vector &source) {
	
	//-- Make sure space is allocated.
	target.resize(source.size() * 2, 0.0);

	//-- Get the length of the source vector.
	int full_length = source.size();
	int half_length = full_length / 2;

	for (int index = 0; index < half_length; index++) {
		target[half_length + index]     = source[index];
		target[half_length - index - 1] = source[index];
	}

	for (int index = half_length; index < full_length; index++) {
		target[ half_length + index] = source[index];
		target[(full_length+full_length+half_length-1) - index] = source[index];
	}
}





yarp::sig::Vector getSampleDelay() {

	double micDistance  = 0.145;
	double C            = 336.628;
	int    samplingRate = 48000;

	int angle_resolution = 180;
	int startAngle_index =   0 * (angle_resolution / 180);
	int endAngle_index   = 180 * (angle_resolution / 180);
	int half_angle_res   = angle_resolution / 2;

	yarp::sig::Vector sample_delay(angle_resolution, 0.0);

	for (int angle = startAngle_index; angle < endAngle_index; angle++) {
		sample_delay[angle] = 180.0 * angle / (angle_resolution-1) * M_PI / 180.0;
		sample_delay[angle] = micDistance * samplingRate * (-cos(sample_delay[angle])) / C;
		sample_delay[angle] = myRound(sample_delay[angle]);		
		
		if (angle) sample_delay[angle] -= sample_delay[0]; //-- Normalize based on first index.
	} sample_delay[0] -= sample_delay[0];

	return sample_delay;
}



int main() {

	//-- Header.
	std::cout << "\n\n";
	std::cout << "###############################\n";
	std::cout << "#                             #\n";
	std::cout << "#  Spline-Interpolation Demo  #\n";
	std::cout << "#                             #\n";
	std::cout << "###############################\n\n";

	yarp::os::SystemClock myTime; 
	double now;
	double then;
	int temp_size = 10000000;
	
	yarp::sig::Vector temp_src(temp_size), temp_target;
	for (int i = 0; i < temp_size; i++) {
		temp_src[i] = i+1;
	}

	//yarpPrintVector(temp_src);
	
	now = myTime.now();
	frontFieldMirror(temp_target, temp_src);
	then = myTime.now();
	std::cout << "Time for OG:  " << then - now << std::endl;

	//yarpPrintVector(temp_target);

	now = myTime.now();
	frontFieldMirror_alt(temp_target, temp_src);
	then = myTime.now();
	std::cout << "Time for New: " << then - now << std::endl;

	//yarpPrintVector(temp_target);

	now = myTime.now();
	frontFieldMirror(temp_target, temp_src);
	then = myTime.now();
	std::cout << "Time for OG:  " << then - now << std::endl;

	

	return 0;

	//-- 
	//-- Variable Set up.
	//--

	int    nMics        = 2;
	double C            = 336.628;
	double micDistance  = 0.145;
	const double pi     = 2 * acos(0.0);

	int frameSamples    = 4096;
	int samplingRate    = 48000;

	//-- Take the ceiling of of (D/C)/Rate.
	//--   ceiling = (x + y - 1) / y
	int nBeamsPerHemifield = ((micDistance * samplingRate) + C - 1.0) / C;
	int nBeams = 2 * nBeamsPerHemifield + 1;

	_print << "Speed of Sound"      << ": " << C << "\n";
	_print << "Distance of Mic"     << ": " << micDistance << "\n";
	_print << "Sampling Rate"       << ": " << samplingRate << "\n";
	_print << "Frame Samples"       << ": " << frameSamples << "\n\n";
	_print << "Beams Per Hemifield" << ": " << nBeamsPerHemifield << "\n";
	_print << "Total Number Beams " << ": " << nBeams << "\n\n";


	//--
	//-- Set up Lags.
	//--

	yarp::sig::Vector lags(nBeams);
	for (int i = 0; i < lags.size(); i++) {
		lags[i] = (-nBeamsPerHemifield + i) * (C / samplingRate);
	}

	_print << "Number of Lags" << ": " << lags.size() << "\n";
	if (printLags) {
		_print << "\n Lags " << lags.size() << ": \n"; 
		yarpPrintVector(lags);	
	}


	//-- 
	//-- Set up Angles.
	//-- 

	yarp::sig::Vector angles(nBeams);
	for (int i = 0; i < angles.size(); i++) {
		angles[i] = (1.0 / micDistance) * (-nBeamsPerHemifield + i) * (C / samplingRate);
		angles[i] = (angles[i] <= -1.0) ? -1.0 : angles[i];
		angles[i] = (angles[i] >=  1.0) ?  1.0 : angles[i];

		angles[i] = asin(angles[i]);
	}

	_print << "Number of Angles" << ": " << angles.size() << "\n";
	if (printAngles) { 
		_print << "\n Angles " << angles.size() << ": \n";
		yarpPrintVector(angles);	
	}


	//--
	//-- Set up Space Angles.
	//--
	
	int    radialResolution_degrees = 1;
	double radialResolution_radians = pi/180.0 * radialResolution_degrees;
	int    numSpaceAngles           = 360 / radialResolution_degrees;

	double linspace_step = ((pi - radialResolution_radians) - (-pi)) / (numSpaceAngles - 1.0);
	double current_step  = -pi;
	
	_print << "Radial Res Degrees" << ": " << radialResolution_degrees << "\n";
	_print << "Radial Res Radians" << ": " << radialResolution_radians << "\n";
	_print << "Number Space Angle" << ": " << numSpaceAngles << "\n";
	_print << "Step Size"          << ": " << linspace_step << "\n";
	
	yarp::sig::Vector spaceAngles(numSpaceAngles);

	for (int i = 0; i < spaceAngles.size(); i++) {
		spaceAngles[i] = current_step;
		current_step += linspace_step;
	}
	if (printSpaceAng) { 
		_print << "\n Space Angles " << spaceAngles.size() << ": \n";
		yarpPrintVector(spaceAngles);
	}

	//-- 
	//-- Set up Normal Angles.
	//--
	int numNormalAngles = numSpaceAngles / 2;
	linspace_step = (((pi/2) - radialResolution_radians) - (-pi/2)) / (numNormalAngles - 1.0);
	current_step  = -pi/2;
	yarp::sig::Vector normalAngles(numNormalAngles);

	for (int i = 0; i < normalAngles.size(); i++) {
		normalAngles[i] = current_step;
		current_step += linspace_step;
	}
	if (printNormalAng) { 
		_print << "\n Normal Angles " << normalAngles.size() << ": \n";
		yarpPrintVector(normalAngles); 
	}

	yarp::sig::Vector trueNormalAngle;
	frontFieldMirror(trueNormalAngle, normalAngles);

	if (printTrueNormalAng) {
		_print << "\n True Normal Angles " << trueNormalAngle.size() << ": \n";
		yarpPrintVector(trueNormalAngle);
	}



	//--
	//-- Set up Mic Angles.
	//--

	yarp::sig::Vector micAngles(nBeams*2 - 2);

	//-- Concatenate angles[:] with angles[1:-1]+pi.
	for (int i = 0; i < angles.size(); i++) {
		micAngles[i] = angles[i];
	}
	
	for (int i = 0; i < angles.size()-2; i++) {
		micAngles[i+angles.size()] = angles[i+1] + pi;
	}
	
	//-- Roll the vector.
	micAngles = yarpRollVector(micAngles, nBeamsPerHemifield);

	//-- Unwrap the vector.
	micAngles = yarpUnwrap(micAngles);

	//-- Normalize to +/- pi.
	for (int i = 0; i < micAngles.size(); i++) {
		micAngles[i] -= (2*pi);
	}
	if (printMicAng) { 
		_print << "\n Mic Angles " << micAngles.size() << ": \n";
		yarpPrintVector(micAngles); 
	}

	yarp::sig::Vector angle_index = getSampleDelay();

	if (printAngleIndex) {
		_print << "\n Angle Index " << angle_index.size() << ": \n";
		yarpPrintVector(angle_index);
	}


	//-- Generate a Frequency and beam form it.
	yarp::sig::Vector egoSpaceMap_front(nBeams);
	yarp::sig::Vector egoSpaceMap_loRes(nBeams*2-2);

	double freq_lin_step = (frameSamples * 2 * pi / samplingRate) / (frameSamples - 1.0);
	double freq_cur_step = 0.0;
	double freq = 3000;

	
	yarp::sig::Vector left(frameSamples);
	for (int i = 0; i < left.size(); i++) {
		left[i] = sin(freq_cur_step * freq);
		freq_cur_step += freq_lin_step;
	}

	yarp::sig::Vector right(left);
	
	for (int beam = 0; beam < nBeams; beam++) {
		
		egoSpaceMap_front[beam] = 0.0;

		for (int frame = 0; frame < frameSamples; frame++) {
			egoSpaceMap_front[beam] += pow(left[frame] + left[myMod(frame + (nBeamsPerHemifield - beam), frameSamples)], 2);
		}
		egoSpaceMap_front[beam] = sqrt(egoSpaceMap_front[beam] / ((double)frameSamples));

	}


	if (printEgoMapFront) {
		_print << "\n Ego Map Front Field " << egoSpaceMap_front.size() << ": \n";
		yarpPrintVector(egoSpaceMap_front); 
	}

	egoSpaceMap_loRes = setLowResolutionMap(egoSpaceMap_front);
	

	if (printEgoMapLo) { 
		_print << "\n Ego Map Low Resolution " << egoSpaceMap_loRes.size() << ": \n";
		yarpPrintVector(egoSpaceMap_loRes); 
	}


	

	
	yarp::sig::Vector egoSpaceMap_hiRes(numSpaceAngles);
	yarp::sig::Vector egoSpaceMap_hiRes_2(180);


	int idx0, idx1;

	for (int pos = 0; pos < 180; pos++) {
		
		if (normalAngles[pos] < angles[angle_index[pos]]) {
			idx0 = angle_index[pos] - 1;
			idx1 = angle_index[pos];
		} else {
			idx0 = angle_index[pos];
			idx1 = angle_index[pos] + 1;
		}

		egoSpaceMap_hiRes_2[pos] = lininterp (
			normalAngles[pos],
			angles[idx0],
			egoSpaceMap_front[idx0],
			angles[idx1],
			egoSpaceMap_front[idx1]
		);
		
		if (pos % 90 == 0) {std::cout << "\n SWITCH \n\n"; }
		std::cout << std::right << std::setw(4) << pos << " [" << std::setw(2) << angle_index[pos] << "] :  "  
			<< std::setw(10) << angles[idx0] << "   <= "
			<< std::setw(10) << normalAngles[pos] << "   <= "
			<< std::setw(10) << angles[idx1] << "  "
		//	<< std::setw(12) << angles[angle_index[pos]] << "  "
		//	<< std::setw(12) << egoSpaceMap_front[angle_index[pos]] 
		//	<< "  " << (normalAngles[pos] < angles[angle_index[pos]])
			<< "  " << "(" << idx0 << ", " << idx1 << ")       --> "
			<< std::setw(12) << egoSpaceMap_hiRes_2[pos]
			<< "\n";

		

		

		
	}




	return 0;

	int k = 1;
	for (int i = 0; i < numSpaceAngles; i++) {
		if (spaceAngles[i] > micAngles[k] && k < micAngles.size()-1) {
			k++;
		}
		egoSpaceMap_hiRes[i] = lininterp (
			spaceAngles[i],           // x
			micAngles[k-1],           // x1 
			egoSpaceMap_loRes[k-1],   // y1
			micAngles[k],             // x2
			egoSpaceMap_loRes[k]      // y2
		);
	}

	std::cout << k << " " << micAngles.size() << std::endl;

	for (int i = 0; i < 20; i++) {
		if (i % 6 == 0) std::cout << std::endl;
		std::cout << egoSpaceMap_hiRes[i] << " ";
	} std::cout << "\n\n";

	for (int i = 360-20; i < 360; i++) {
		if (i % 6 == 0) std::cout << std::endl;
		std::cout << egoSpaceMap_hiRes[i] << " ";
	} std::cout << "\n";

	return 0;
}
