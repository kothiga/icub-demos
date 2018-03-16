/*
 * @file headTurnModule.h
 * @brief Simple module to pan the iCub head back and forth.
 */

#ifndef _HEAD_TURN_MODULE_H_
#define _HEAD_TURN_MODULE_H_

#include <iostream>
#include <string>

#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Network.h>
#include <yarp/os/Thread.h>
#include <yarp/os/Log.h>

#include <iCub/headTurnRatethread.h>

class headTurnModule : public yarp::os::RFModule {

  std::string moduleName;                  // name of the module
  std::string robotName;                   // name of the robot
  std::string robotPortName;               // name of robot port
  std::string inputPortName;               // name of the input port for events
  std::string outputPortName;              // name of output port
  std::string handlerPortName;             // name of handler port
  std::string configFile;                  // name of the configFile that the resource Finder will seek

  yarp::os::Port handlerPort;              // a port to handle messages

  headTurnRatethread *rThread;

public:
  /*
   * configure all the parameters and return true if successful
   * @param rf reference to the resource finder
   * @return flag for the success
   */
  bool configure(yarp::os::ResourceFinder &rf);

  /*
   *  interrupt, e.g., the ports
   */
  bool interruptModule();

  /*
   *  close and shut down the module
   */
  bool close();

  /*
   *  to respond through rpc port
   * @param command reference to bottle given to rpc port of module, alongwith parameters
   * @param reply reference to bottle returned by the rpc port in response to command
   * @return bool flag for the success of response else termination of module
   */
  bool respond(const yarp::os::Bottle& command, yarp::os::Bottle& reply);

  /*
   *  unimplemented
   */
  double getPeriod();

  /**
   *  unimplemented
   */
   bool updateModule();
};


#endif // _HEAD_TURN_MODULE_H_

//----- end-of-file --- ( next line intentionally left blank ) ------------------
