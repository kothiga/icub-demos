/*
 * @file headTurnRatethread.cpp
 * @brief Implementation of the headTurnRatethread (see header file).
 */

#include <iCub/headTurnRatethread.h>

using namespace yarp::dev;
using namespace yarp::os;
using namespace yarp::sig;
using namespace std;

/*
 * Turn the iCub head back and forth using PolyDriver
 */

#define THRATE 10 //ms

headTurnRatethread::headTurnRatethread() : RateThread(THRATE) {
  robot = "icub";
}

headTurnRatethread::headTurnRatethread(string _robot, string _configFile) : RateThread(THRATE) {
  robot = _robot;
  configFile = _configFile;
}

headTurnRatethread::~headTurnRatethread() {
  delete robotHead;
  delete pos;
  delete vel;
  delete enc;
}

bool headTurnRatethread::threadInit() {

  // set up polydriver with head
  options.put("device", "remote_controlboard");
  options.put("local", getName("local_controlboard"));
  options.put("remote", getName("/head").c_str());

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
  setpoints.resize(jnts);
  checkpoints.resize(jnts);

  //currentpos = 0;
  //counter = 100;

  // init speed
  speeds[0] = 20;
  speeds[1] = 20;
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

void headTurnRatethread::setName(string str) {
  this->name=str;
}


std::string headTurnRatethread::getName(const char* p) {
  string str(name);
  str.append(p);
  return str;
}

void headTurnRatethread::setInputPortName(string InpPort) {

}

void headTurnRatethread::run() {
  double yaw = position[count];
  result = processing();
  setpoints[0] = yaw;
  //pos->positionMove(setpoints.data());

  vel->velocityMove(setpoints.data());


  // update counter
  count = (count+1) % 2;
}

bool headTurnRatethread::processing() {
  // here goes the processing...
  return true;
}

void headTurnRatethread::threadRelease() {
  inputPort.interrupt();
  outputPort.interrupt();
  inputPort.close();
  outputPort.close();
}
