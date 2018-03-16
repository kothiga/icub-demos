/*
 * @file redBallDemoRatethread.cpp
 * @brief Implementation of the headTurnRatethread (see header file).
 */

#include <iCub/redBallDemoRatethread.h>

using namespace yarp::dev;
using namespace yarp::os;
using namespace yarp::sig;
using namespace std;

/*
 * Turn the iCub head back and forth using PolyDriver
 */

#define THRATE 10 //ms

redBallDemoRatethread::redBallDemoRatethread() : RateThread(THRATE) {
  robot = "icub";
}

redBallDemoRatethread::redBallDemoRatethread(string _robot, string _configFile) : RateThread(THRATE) {
  robot = _robot;
  configFile = _configFile;
}

redBallDemoRatethread::~redBallDemoRatethread() {
  delete robotHead;
  delete pos;
  delete vel;
  delete enc;
  delete image;
  delete outputImage;
}

bool redBallDemoRatethread::threadInit() {

  // set up polydriver with head
  options.put("device", "remote_controlboard");
  options.put("local", getName("local_controlboard"));
  options.put("remote", "/icub/head");

  // init robot
  robotHead = new PolyDriver(options);
  if (!robotHead->isValid()) {
    yInfo("Cannot connect to robot head\n");
    return 1;
  }

  // get robot interfaces
  robotHead->view(pos);
  robotHead->view(vel);
  robotHead->view(enc);

  if (pos==NULL || vel==NULL || enc==NULL) {
    yInfo("Cannot get interface to robot head\n");
    robotHead->close();
    return false;
  }

  // get total available axes
  int jnts = 0;
  pos->getAxes(&jnts);

  // resize these to axes
  encoder.resize(jnts);
  setpoints.resize(jnts);
  checkpoints.resize(jnts);

  // init speed
  speeds[0] = 15;
  speeds[1] = 15;
  pos->setRefSpeeds(speeds);

  flagger[2];

  // init points
  setpoints[1] = 0;
  setpoints[0] = 0;
  currentpos = 0.0;

  // init position
  pos->positionMove(setpoints.data());
  yInfo("Robot Head initialized correctly.");


  if (!inputPort.open(getName("/img:i").c_str())) {
    yError("unable to open port to receive images");
    return false;
  }

  if (!outputPort.open(getName("/img:o").c_str())) {
    yError("unable to open port to send images");
    return false;
  }

  yInfo("Initialization of the processing thread correctly ended");
  return true;
}

void redBallDemoRatethread::setName(string str) {
  this->name=str;
}


std::string redBallDemoRatethread::getName(const char* p) {
  string str(name);
  str.append(p);
  return str;
}

void redBallDemoRatethread::setInputPortName(string InpPort) {

}

void redBallDemoRatethread::run() {

  //-- don't do work if no one is connected
  if(inputPort.getInputCount()) {
    //-- grab the image
    image = inputPort.read(false);
    //-- verify we got something
    if (image != NULL) {
      //-- only do work if we are sending an image
      if (outputPort.getOutputCount()) {

        //-- begin processing
        outputImage = &outputPort.prepare();
        outputImage->resize(image->width(),image->height());
        result = processing();
        outputPort.write();

      }
    }
  }
}

bool redBallDemoRatethread::processing() {

  xMean = 0.0;
  yMean = 0.0;
  ct = 0;
  int width  = image->width();
  int height = image->height();

  //-- find all the reddish pixels in our
  //-- threshold and average them
  for (int x = 0; x < height; x++) {
    for (int y = 0; y < width; y++) {

      PixelRgb& pixel = image->pixel(y, x);
      //if (pixel.r > pixel.b*1.7+10 && pixel.r > pixel.g*1.7+10) {
      if (pixel.r > 175 && pixel.b < 80 && pixel.g < 80) {
        xMean += x;
        yMean += y;
        ct++;
      }
    }
  }

  //-- grab the average of red pixel coordinates
  if (ct > 0) {
    xMean /= ct;
    yMean /= ct;
  }

  //-- if the number of red pixels is above
  //-- our threshold, draw on the image
  if (ct > (image->width()/30)*(image->height()/30)) {

    unsigned char* pImage  = outputImage->getRawImage();
    unsigned char* inImage = image->getRawImage();

    for (int x = 0; x < height; x++) {
      for (int y = 0; y < width; y++) {

        //-- if we are in range of the
        //-- mean of red pixels
        if (std::abs(xMean-x) < 5 && std::abs(yMean-y) < 5) {

          //-- Red Pixel Value
          *pImage = 0;
          pImage++; inImage++;

          //-- Green Pixel Value
          *pImage = 255;
          pImage++; inImage++;

          //-- Blue Pixel Value
          *pImage = 0;
          pImage++; inImage++;

        } else {

          //-- Red Pixel Value
          *pImage = *inImage;
          pImage++; inImage++;

          //-- Green Pixel Value
          *pImage = *inImage;
          pImage++; inImage++;

          //-- Blue Pixel Value
          *pImage = *inImage;
          pImage++; inImage++;
        }
      }
    }

    //-- move the head to center
    //-- the red object
    enc->getEncoders(encoder.data());
    double yaw = ((((width/2) - yMean)*20) / width) + encoder[0]+2;

    printf("Encoder at %g\n", encoder[0]);

    if (std::abs(currentpos-encoder[0]) < 0.5 || std::abs(yaw) > 38) {

      printf("Moving to %g\n", yaw);

      currentpos = yaw;
      setpoints[0] = yaw;
      pos->positionMove(setpoints.data());

    }

  } else {

      //-- just copy the image over
      unsigned char* pImage  = outputImage->getRawImage();
      unsigned char* inImage = image->getRawImage();

      for (int x = 0; x < height; x++) {
        for (int y = 0; y < width; y++) {
          //-- Red Pixel Value
          *pImage = *inImage;
          pImage++; inImage++;

          //-- Green Pixel Value
          *pImage = *inImage;
          pImage++; inImage++;

          //-- Blue Pixel Value
          *pImage = *inImage;
          pImage++; inImage++;
        }
    }
  }

  return true;
}

void redBallDemoRatethread::threadRelease() {
  //-- stop the head
  robotHead->close();

  //-- interrupt the ports
  inputPort.interrupt();
  outputPort.interrupt();

  //-- close the ports
  inputPort.close();
  outputPort.close();
}
