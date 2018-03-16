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
}

bool redBallDemoRatethread::threadInit() {

  // set up polydriver with head
  options.put("device", "remote_controlboard");
  options.put("local", getName("local_controlboard"));
  options.put("remote", "/icub/head");

  robotHead = new PolyDriver(options);
  if (!robotHead->isValid()) {
    yInfo("Cannot connect to robot head\n");
    return 1;
  }

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
  speeds[0] = 10;
  speeds[1] = 10;
  pos->setRefSpeeds(speeds);

  flagger[2];

  // init points
  setpoints[1] = 0;
  setpoints[0] = 0;

  // init position
  pos->positionMove(setpoints.data());

  // set move locations
  position[0] =  40;
  position[1] = -40;
  count = 0;

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

  while (true) {

  }
}

bool redBallDemoRatethread::processing() {
  // here goes the processing...
  return true;
}

void redBallDemoRatethread::threadRelease() {
  robotHead->close();
}
