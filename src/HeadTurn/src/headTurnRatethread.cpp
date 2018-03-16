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

  pos->positionMove(setpoints.data());

  while (true) {

    //-- get information on the
    //-- encoders current position
    enc->getEncoders(encoder.data());
    yInfo("Yaw at %lf).", encoder[0]);

    //-- wait for the joint to make its way
    //-- to the location, before giving
    //-- another instruction
    if (std::abs(yaw-encoder[0]) < 0.1) {
      break;
    }

    //-- sleep for a bit
    usleep(20000);
  }

  //-- update counter
  count = (count+1) % 2;

}

bool headTurnRatethread::processing() {
  // here goes the processing...
  return true;
}

void headTurnRatethread::threadRelease() {
  robotHead->close();
}
