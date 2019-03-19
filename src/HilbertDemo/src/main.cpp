#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <time.h>

#include <fftw3.h>
#include <vector>

#include <yarp/math/Math.h>
#include <yarp/sig/all.h>
#include <yarp/os/all.h>


void loadRaw(yarp::sig::Matrix& source, const std::string path, const int Chan, const int Samp) {

    source.resize(Chan, Samp);
    source.zero();

    std::ifstream reader(path);

    for (int c = 0; c < Chan; c++) {
        for (int s = 0; s < Samp; s++) {
            reader >> source[c][s];
        }
    }
}


std::string MatrixToString(const yarp::sig::Matrix& source, const int precision, const int width, const char* endRowStr) {

    //-- Get some information of the source.
    const size_t RowSize = source.rows();
    const size_t ColSize = source.cols();
    size_t row, col;

    //-- If the width is less than 1, use tabs, else use width number of spaces.
    std::string spacer((width<0) ? 1 : width, (width<0) ? '\t' : ' ');

    //-- Buffering.
    std::string target = "";
    char buffer[350]; 
    const double* src = source.data();

    //-- Iterate through copying the contents
    //-- from the matrix, into a string.
    //-- Avoid unnecessary string resizes by only
    //-- adding spacers at the beginning.
    for (row = 0; row < RowSize; row++) {
        if (row) { target += endRowStr; }
        for (col = 0; col < ColSize; col++) {
            if (col) { target += spacer; }
            sprintf(buffer, "%.*lf", precision, *src); src++;
            target += buffer;
        }
    }

    return target;
}


void MatrixToFile(const yarp::sig::Matrix& source, const std::string fileName) {

    std::ofstream writer(fileName);
    writer << MatrixToString(source, 10, 1, "\n");
    writer.close();
}


inline void RealToComplex(const double* source, fftw_complex* target, const size_t numSamples) {

    const int REAL=0, IMAG=1;

    //-- Iterate through and ``drop`` the negative half of the frequency
    //-- spectrum, effectively turning the signal into a complex one.
    for (size_t idx = 0; idx < numSamples; idx++) {
        target[idx][REAL] = source[idx];
        target[idx][IMAG] = 0.0;
    }
}


inline double c2r(const fftw_complex x) {

    const int REAL=0, IMAG=1;

    //-- u = sqrt( ur^2 + ui^2 )
    return sqrt( x[REAL]*x[REAL] + x[IMAG]*x[IMAG] );
}


inline void ComplexToReal(const fftw_complex* source, double* target, const size_t numSamples) {

    //-- Iterate through and convert the data to a real number.
    for (size_t idx = 0; idx < numSamples; idx++) {
        target[idx] = c2r(source[idx]);
    }
}


void singleAnalyticalSignal(const fftw_complex* source, fftw_complex* target, const size_t numSamples, fftw_plan& forward, fftw_plan& backward) {

    const int REAL=0, IMAG=1;

    //-- Get some information of the signals length.
    size_t halfNumSamples = numSamples >> 1;
    size_t numRemaining   = halfNumSamples;

    //-- Perform fft on the source.
    fftw_execute(forward);

    //-- First half of the signals imaginary and real 
    //-- parts are doubled. 
    for (size_t idx = 1; idx < halfNumSamples; idx++) {
        target[idx][REAL] *= 2.0;
        target[idx][IMAG] *= 2.0;
    }

    //-- One less remainder when even number of samples.
    if (numSamples % 2 == 0) {
        numRemaining--;
    }

    //-- If the number of samples was odd, and 
    //-- greator than one, double the half position too.
    else if (numSamples > 1) {
        target[halfNumSamples][REAL] *= 2.0;
        target[halfNumSamples][IMAG] *= 2.0;
    }

    //-- Drop the second half of the signal.
    memset(&target[halfNumSamples+1][REAL], 0.0, numRemaining * sizeof(fftw_complex));

    //-- Perform ifft on the target.
    fftw_execute(backward);

    //-- Scale the idft output.
    for (size_t idx = 0; idx < numSamples; idx++) {
        target[idx][REAL] /= numSamples;
        target[idx][IMAG] /= numSamples;
    }
}



int main() {

    const std::string path = "/home/austin/Temp/raw.data";
    const std::string save = "/home/austin/Temp/out.data";
    const int Samples  = 753664; 
    const int Channels = 2;


    std::cout << "Reading File: " << path << "\n";
    yarp::sig::Matrix source, target;

    source.resize(Channels, Samples);
    source.zero();

    target.resize(Channels, Samples);
    target.zero();

    loadRaw(source, path, Channels, Samples);

    std::cout << "Setting Up FFTW. \n";
    fftw_complex* fft_input_0;
	fftw_complex* fft_output_0;
    fft_input_0  = (fftw_complex *) fftw_malloc(Samples * sizeof(fftw_complex));
    fft_output_0 = (fftw_complex *) fftw_malloc(Samples * sizeof(fftw_complex));
    fftw_plan forward_0  = fftw_plan_dft_1d(Samples, fft_input_0,  fft_output_0, FFTW_FORWARD,  FFTW_ESTIMATE);
    fftw_plan backward_0 = fftw_plan_dft_1d(Samples, fft_output_0, fft_output_0, FFTW_BACKWARD, FFTW_ESTIMATE);

    fftw_complex* fft_input_1;
	fftw_complex* fft_output_1;
    fft_input_1  = (fftw_complex *) fftw_malloc(Samples * sizeof(fftw_complex));
    fft_output_1 = (fftw_complex *) fftw_malloc(Samples * sizeof(fftw_complex));
    fftw_plan forward_1  = fftw_plan_dft_1d(Samples, fft_input_1,  fft_output_1, FFTW_FORWARD,  FFTW_ESTIMATE);
    fftw_plan backward_1 = fftw_plan_dft_1d(Samples, fft_output_1, fft_output_1, FFTW_BACKWARD, FFTW_ESTIMATE);

    RealToComplex(source[0], fft_input_0, Samples);
    RealToComplex(source[1], fft_input_1, Samples);
    
    std::cout << "Running FFT. \n";
    singleAnalyticalSignal(fft_input_0, fft_output_0, Samples, forward_0, backward_0);
    singleAnalyticalSignal(fft_input_1, fft_output_1, Samples, forward_1, backward_1);

    ComplexToReal(fft_output_0, target[0], Samples);
    ComplexToReal(fft_output_1, target[1], Samples);

    std:: cout << "Saving to File. \n";
    MatrixToFile(target, save);

    std::cout << "Cleaning Up. \n";
    fftw_free(fft_input_0 );
    fftw_free(fft_output_0);
    fftw_free(fft_input_1 );
    fftw_free(fft_output_1);

    fftw_destroy_plan(forward_0 );
    fftw_destroy_plan(backward_0);
    fftw_destroy_plan(forward_1 );
    fftw_destroy_plan(backward_1);

    fftw_cleanup();

    return 0;
}