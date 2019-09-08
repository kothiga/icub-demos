/*
* @file redBallDemoRatethread.h
* @brief Definition of a thread that receives an image from input port tracks a red ball.
*/

#ifndef _RED_BALL_DEMO_RATETHREAD_H_
#define _RED_BALL_DEMO_RATETHREAD_H_

#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <time.h>

#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/dev/all.h>
#include <yarp/os/RateThread.h> 
#include <yarp/os/Log.h>

class redBallDemoRatethread : public yarp::os::RateThread {

private:

  bool result;                    //result of the processing

  std::string robot;              // name of the robot
  std::string configFile;         // name of the configFile where the parameter of the camera are set
  std::string inputPortName;      // name of input port for incoming events, typically from aexGrabber
  std::string name;               // rootname of all the ports opened by this thread

  // polydriver
  yarp::os::Property options;
  yarp::dev::PolyDriver *robotHead;

  yarp::dev::IPositionControl *pos;
  yarp::dev::IVelocityControl *vel;
  yarp::dev::IEncoders *enc;

  yarp::sig::Vector encoder;
  yarp::sig::Vector setpoints;
  yarp::sig::Vector checkpoints;

  //int count;
  double currentpos;
  double counter;
  double speeds[2];
  double position[2];
  bool flagger[2];

  // image proc
  yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > inputPort;
  yarp::os::BufferedPort<yarp::sig::ImageOf<yarp::sig::PixelRgb> > outputPort;
  yarp::sig::ImageOf<yarp::sig::PixelRgb>* image;
  yarp::sig::ImageOf<yarp::sig::PixelRgb>* outputImage;

  double xMean;
  double yMean;
  int ct;

public:
  /*
   * constructor default
   */
  redBallDemoRatethread();

  /*
   * constructor
   * @param robotname name of the robot
   */
  redBallDemoRatethread(std::string robotname, std::string configFile);

  /*
   * destructor
   */
  ~redBallDemoRatethread();

  /*
   * initialises the thread
   */
  bool threadInit();

  /*
   * correctly releases the thread
   */
  void threadRelease();

  /*
   * active part of the thread
   */
  void run();

  /*
   * function that sets the rootname of all the ports that are going to be created by the thread
   * @param str rootnma
   */
  void setName(std::string str);

  /*
   * function that returns the original root name and appends another string iff passed as parameter
   * @param p pointer to the string that has to be added
   * @return rootname
   */
  std::string getName(const char* p);

  /*
   * function that sets the inputPort name
   */
  void setInputPortName(std::string inpPrtName);

  /*
   * method for the processing in the ratethread
   * @param mat matrix to be processed in the method
   */
  bool processing();
};

#endif  //_RED_BALL_DEMO_RATETHREAD_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------
