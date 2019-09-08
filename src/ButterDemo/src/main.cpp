#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <time.h>

#include <vector>

#include <yarp/math/Math.h>
#include <yarp/sig/all.h>
#include <yarp/os/all.h>

#include <iCub/butterworth.h>

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


int main() {

    const std::string path     = "/home/austin/temp/raw.data";
    const std::string save_low = "/home/austin/temp/butter_low_10hz.data";
    const std::string save_bnd = "/home/austin/temp/butter_bnd_5hz.data";

    const int Samples  = 753664; 
    const int Channels = 2;
    const int Rate     = 48000;
    
    const int Width    = 1;

    std::cout << "Reading File: " << path << "\n";
    yarp::sig::Matrix source, target_low, target_bnd;

    source.resize(Channels, Samples);
    source.zero();

    target_low.resize(Channels, Samples);
    target_low.zero();

    target_bnd.resize(Channels, Samples);
    target_bnd.zero();

    loadRaw(source, path, Channels, Samples);

    Filters::Butterworth butter(Rate, Width);

    std::cout << "Filtering . . . \n";
    butter.getLowPassedAudio(source, target_low, 10.0);
    butter.getBandPassedAudio(source, target_bnd, 5.0);

    std:: cout << "Saving to File. \n";
    MatrixToFile(target_low, save_low);
    MatrixToFile(target_bnd, save_bnd);

    return 0;
}