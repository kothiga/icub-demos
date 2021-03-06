// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
  * Copyright (C) 2018 Department of Neuroscience - University of Lethbridge
  * Author: Austin Kothig, Francesco Rea, Marko Ilievski, Matt Tata
  * email: kothiga@uleth.ca, francesco.reak@iit.it, marko.ilievski@uwaterloo.ca, matthew.tata@uleth.ca
  * 
  * Permission is granted to copy, distribute, and/or modify this program
  * under the terms of the GNU General Public License, version 2 or any
  * later version published by the Free Software Foundation.
  *
  * A copy of the license can be found at
  * http://www.robotcub.org/icub/license/gpl.txt
  *
  * This program is distributed in the hope that it will be useful, but
  * WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
  * Public License for more details
*/

/* ===========================================================================
 * @file  tutorialModule.cpp
 * @brief Implementation of the tutorialModule (see header file).
 * =========================================================================== */

#include <iCub/tutorialModule.h>

bool tutorialModule::configure(yarp::os::ResourceFinder &rf) {
    
    /* ===========================================================================
     *  Process all parameters from both command-line and .ini file.
     * =========================================================================== */

    //-- Get the module name which will form the stem of all module port names.
    moduleName = rf.check ("name", yarp::os::Value("/tutorial"), "module name (string)").asString();


    /* ===========================================================================
     *  Before continuing, set the module name before getting any other parameters, 
     *  specifically the port names which are dependent on the module name.
     * =========================================================================== */
    setName(moduleName.c_str());


    /* ===========================================================================
     *  Get the robot name which will form the stem of the robot ports names
     *  and append the specific part and device required.
     * =========================================================================== */
    robotName = rf.check ("robot", yarp::os::Value("icub"), "Robot name (string)").asString();
    robotPortName = "/" + robotName + "/head";

    inputPortName = rf.check("inputPortName", yarp::os::Value(":i"), "Input port name (string)").asString();
    
    
    /* ===========================================================================
     *  Attach a port of the same name as the module (prefixed with a /) to the module
     *  so that messages received from the port are redirected to the respond method.
     * =========================================================================== */
    handlerPortName  = "";
    handlerPortName += getName(); //-- Use getName() rather than a literal.

    if (!handlerPort.open(handlerPortName.c_str())) {           
        yInfo("%s: Unable to open port %s", getName().c_str(), handlerPortName.c_str());
        return false;
    }

    attach(handlerPort); //-- Attach to port.
    if (rf.check("config")) {
        configFile = rf.findFile(rf.find("config").asString().c_str());
        if (configFile == "") {
            return false;
        }
    } else {
        configFile.clear();
    }


    /* =========================================================================== 
     *  Create the thread and pass pointers to the module parameters.
     * =========================================================================== */
    pThread = new tutorialPeriodicThread(robotName, configFile);
    pThread->setName(getName().c_str());
    

    /* ===========================================================================
     *  Now start the thread to do the work. 
     * =========================================================================== */
    pThread->start();


    //-- Let the RFModule know everything went 
    //-- well so that it will then run the module.
    return true ;     
}


bool tutorialModule::interruptModule() {
    handlerPort.interrupt();
    return true;
}


bool tutorialModule::close() {

    handlerPort.close();

    //-- Stop the thread.
    yDebug("Stopping the thread . . . \n");
    pThread->stop();

    //-- Release the periodic thread.
    delete pThread;

    return true;
}


bool tutorialModule::respond(const yarp::os::Bottle& command, yarp::os::Bottle& reply) {

    std::string helpMessage = std::string(getName().c_str()) + " commands are: \n" + "help \n" + "quit \n";
    reply.clear(); 

    if (command.get(0).asString() == "quit") {
        reply.addString("quitting");
        return false;
    } else if (command.get(0).asString() == "help") {
        yInfo(helpMessage.c_str());
        reply.addString("ok");
    }
    
    return true;
}


double tutorialModule::getPeriod() {
    /* Module periodicity (seconds), called implicitly by myModule. */
    return 1;
}


bool tutorialModule::updateModule() {
    /* Called periodically every getPeriod() seconds. */
    return true;
}
