/*
* @file headTurnModule.cpp
* @brief Implementation of the headTurnModule (see header file).
*/

#include <iCub/headTurnModule.h>

using namespace yarp::os;
using namespace yarp::sig;
using namespace std;

/*
 * Configure method. Receive a previously initialized
 * resource finder object. Use it to configure your module.
 * If you are migrating from the old Module, this is the
 *  equivalent of the "open" method.
 */

bool headTurnModule::configure(yarp::os::ResourceFinder &rf) {

  /* Process all parameters from both command-line and .ini file */

  /* get the module name which will form the stem of all module port names */
  moduleName = rf.check("name", Value("/headTurn"), "module name (string)").asString();

  /*
  * before continuing, set the module name before getting any other parameters,
  * specifically the port names which are dependent on the module name
  */
  setName(moduleName.c_str());

  /*
  * get the robot name which will form the stem of the robot ports names
  * and append the specific part and device required
  */
  robotName     = rf.check("robot",         Value("icub"), "Robot name (string)").asString();
  robotPortName = "/" + robotName + "/head";
  inputPortName = rf.check("inputPortName", Value(":i"),   "Input port name (string)").asString();

  /*
  * attach a port of the same name as the module (prefixed with a /) to the module
  * so that messages received from the port are redirected to the respond method
  */
  handlerPortName =  "";
  handlerPortName += getName();         // use getName() rather than a literal

  if (!handlerPort.open(handlerPortName.c_str())) {
    cout << getName() << ": Unable to open port " << handlerPortName << endl;
    return false;
  }

  attach(handlerPort);                  // attach to port
  if (rf.check("config")) {
    configFile=rf.findFile(rf.find("config").asString().c_str());
    if (configFile=="") {
      return false;
    }
  } else {
    configFile.clear();
  }


  /* create the thread and pass pointers to the module parameters */
  rThread = new freqVisualisationRatethread(robotName, configFile);
  rThread->setName(getName().c_str());
  //rThread->setInputPortName(inputPortName.c_str());

  /* now start the thread to do the work */
  rThread->start();   // this calls threadInit() and it if returns true, it then calls run()

  return true ;       // let the RFModule know everything went well
                      // so that it will then run the module
}

bool headTurnModule::interruptModule() {
  handlerPort.interrupt();
  return true;
}

bool headTurnModule::close() {
  handlerPort.close();
  /* stop the thread */
  yDebug("stopping the thread \n");
  rThread->stop();
  return true;
}

bool headTurnModule::respond(const Bottle& command, Bottle& reply) {
  string helpMessage =  string(getName().c_str()) +
                        " commands are: \n" +
                        "help \n" +
                        "quit \n";
  reply.clear();

  if (command.get(0).asString()=="quit") {

    reply.addString("quitting");
    return false;

  } else if (command.get(0).asString()=="help") {

    cout << helpMessage;
    reply.addString("ok");
  }

  return true;
}

bool headTurnModule::updateModule() {
  /* Called periodically every getPeriod() seconds */
  return true;
}

double headTurnModule::getPeriod() {
/* module periodicity (seconds), called implicitly by myModule */
  return 1;
}
