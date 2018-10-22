#include <iostream>
#include <iomanip>
#include <cmath>

#include <yarp/os/all.h>
#include <yarp/sig/all.h>
#include <yarp/math/Math.h>

#define _print std::cout << std::setw(20) << std::left

inline double HzToErbRate(double Hz) {
    //return (21.4 * log10(4.37e-3 * Hz + 1));
	return (21.4 * log10(4.37e-3 * Hz + 1));
}

inline double ErbRateToHz(double Erb) {
    //return (pow(10, Erb / 21.4) - 1) / 4.37e-3;
	return (pow(10, Erb / 21.4) - 1) / 4.37e-3;
}

#define bw_erb(hz) ( 24.7 * ( 4.37e-3 * hz + 1.0 ) )
#define bw_hz(erb) ( ( ( erb / 24.7 ) - 1.0 ) / 4.37e-3 )

void yarpPrintVector(const yarp::sig::Vector &outVector) {
	for (int i = 0; i < outVector.size(); i++) {
		if (i % 6 == 0) std::cout << std::endl;
			std::cout << std::setw(12) << outVector[i] << " ";    
		} std::cout << "\n\n";
}

inline double myABS  (double a) { return  a >= 0.000001 ? a : ((a) * -1);      }
inline double myRound(double a) { return  a >= 0.000000001 ? ((a - (int)a >= 0.50) ? ((int)a + 1) : ((int)a)) : ((a - (int)a <= -0.50) ? ((int)a-1) : ((int)a)); }


void frontFieldMirror(yarp::sig::Vector &target, yarp::sig::Vector &source) {
	
	//-- Make sure space is allocated.
	target.resize(source.size() * 2, 0.0);

	//-- Get the length of the source vector.
	int length      = source.size();
	int half_length = length / 2;

	//-- Use this for continuous iteration.
	int current_position = 0;

	//-- Mirror First Quarter with Second.
	for (int position = length - half_length - 1; position > -1; position--) {
		target[current_position++] = source[position];
	}

	//-- Set Second and Third Quarter as normal.
	for (int position = 0; position < length; position++) {
		target[current_position++] = source[position];
	}

	//-- Mirror Fourth Quarter with Third.
	for (int position = length - 1; position > length - half_length - 1; position--) {
		target[current_position++] = source[position];
	}
}


void setSampleDelay() {

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

	
	yarpPrintVector(sample_delay);

	yarp::sig::Vector total_delay;
	frontFieldMirror(total_delay, sample_delay);

	yarpPrintVector(total_delay);
}





int main() {

	int minCF  = 80;   //500;
	int maxCF  = 8000; //2800;
	int nBands = 32;

	double minERB = HzToErbRate(minCF);
	double maxERB = HzToErbRate(maxCF);

	double stepSize = (maxERB - minERB) / (nBands - 1);

	yarp::sig::Vector cfs(nBands);

	for (int band = 0; band < nBands; band++) {
		cfs[band] = ErbRateToHz(stepSize * band + minERB);
	}

	yarpPrintVector(cfs);


	setSampleDelay();



	return 0;
}
