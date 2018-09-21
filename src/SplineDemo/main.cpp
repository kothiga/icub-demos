#include <iostream>

using namespace std;

int main() {

  int C               = 338;
  int nMics           = 2;
  double micDistance  = 0.145;
  int frameSamples    = 4096;
  int samplingRate    = 48000;
		
  int nBeamsPerHemi  = (int)((micDistance / C) * samplingRate) - 1;
  int totalBeams = nBeamsPerHemi * 2 + 1;

  cout << int((micDistance / C) * samplingRate) << endl;
  
  return 0;
}
