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
 * @file  tutorialModule.h
 * @brief Simple module as tutorial.
 * =========================================================================== */

#ifndef _TUTORIAL_MODULE_H_
#define _TUTORIAL_MODULE_H_

/** 
 *
 * \defgroup icub_tutorialPeriodicThread tutorialPeriodicThread
 * @ingroup icub_morphoGen
 *
 * This is a module that receives the RGB image from input connection and sends it back to output connection. The purpose
 * of the module is to shift the point of congestion in a network.
 * 
 *
 * 
 * \section lib_sec Libraries
 *
 * YARP.
 *
 * \section parameters_sec Parameters
 * 
 * <b>Command-line Parameters</b> 
 * 
 * The following key-value pairs can be specified as command-line parameters by prefixing \c -- to the key 
 * (e.g. \c --from file.ini. The value part can be changed to suit your needs; the default values are shown below. 
 *
 * - \c from \c tutorialPeriodicThread.ini \n 
 *   specifies the configuration file
 *
 * - \c context \c tutorialPeriodicThread/conf \n
 *   specifies the sub-path from \c $ICUB_ROOT/icub/app to the configuration file
 *
 * - \c name \c tutorialPeriodicThread \n 
 *   specifies the name of the tutorialPeriodicThread (used to form the stem of tutorialPeriodicThread port names)  
 *
 * - \c robot \c icub \n 
 *   specifies the name of the robot (used to form the root of robot port names)
 *
 *
 * <b>Configuration File Parameters</b>
 *
 * The following key-value pairs can be specified as parameters in the configuration file 
 * (they can also be specified as command-line parameters if you so wish). 
 * The value part can be changed to suit your needs; the default values are shown below. 
 *   
 *
 * 
 * \section portsa_sec Ports Accessed
 * 
 * - None
 *                      
 * \section portsc_sec Ports Created
 *
 *  <b>Input ports</b>
 *
 *  - \c /tutorialPeriodicThread \n
 *    This port is used to change the parameters of the tutorialPeriodicThread at run time or stop the tutorialPeriodicThread. \n
 *    The following commands are available
 * 
 *  -  \c help \n
 *  -  \c quit \n
 *
 *    Note that the name of this port mirrors whatever is provided by the \c --name parameter value
 *    The port is attached to the terminal so that you can type in commands and receive replies.
 *    The port can be used by other tutorialPeriodicThreads but also interactively by a user through the yarp rpc directive, viz.: \c yarp \c rpc \c /tutorialPeriodicThread
 *    This opens a connection from a terminal to the port and allows the user to then type in commands and receive replies.
 *       
 *  - \c /tutorialPeriodicThread/image:i \n
 *
 * <b>Output ports</b>
 *
 *  - \c /tutorialPeriodicThread \n
 *    see above
 *
 *  - \c /tutorialPeriodicThread/image:o \n
 *
 * <b>Port types</b>
 *
 * The functional specification only names the ports to be used to communicate with the tutorialPeriodicThread 
 * but doesn't say anything about the data transmitted on the ports. This is defined by the following code. 
 *
 * \c BufferedPort<ImageOf<PixelRgb> >   \c myInputPort; \n 
 * \c BufferedPort<ImageOf<PixelRgb> >   \c myOutputPort;       
 *
 * \section in_files_sec Input Data Files
 *
 * None
 *
 * \section out_data_sec Output Data Files
 *
 * None
 *
 * \section conf_file_sec Configuration Files
 *
 * \c tutorialPeriodicThread.ini  in \c $ICUB_ROOT/app/tutorialPeriodicThread/conf \n
 * \c icubEyes.ini  in \c $ICUB_ROOT/app/tutorialPeriodicThread/conf
 * 
 * \section tested_os_sec Tested OS
 *
 * Linux
 *
 * \section example_sec Example Instantiation of the Module
 * 
 * <tt>tutorialPeriodicThread --name tutorialPeriodicThread --context tutorialPeriodicThread/conf --from tutorialPeriodicThread.ini --robot icub</tt>
 *
 * \author Rea Francesco
 *
 * Copyright (C) 2011 RobotCub Consortium\n
 * CopyPolicy: Released under the terms of the GNU GPL v2.0.\n
 * This file can be edited at \c $ATTENTION_ROOT/src/tutorial/tutorialPeriodicThread/include/iCub/tutorialPeriodicThread.h
 * 
 */

#include <iostream>
#include <string>

#include <yarp/sig/all.h>
#include <yarp/os/all.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/Network.h>
#include <yarp/os/Thread.h>
#include <yarp/os/Log.h>

#include <iCub/tutorialPeriodicThread.h>

class tutorialModule : public yarp::os::RFModule { 

  private:
  
    std::string moduleName;          //-- Name of the module.
    std::string robotName;           //-- Name of the robot .
    std::string robotPortName;       //-- Name of robot port.
    std::string inputPortName;       //-- Name of the input port for events.
    std::string outputPortName;      //-- Name of output port.
    std::string handlerPortName;     //-- Name of handler port.
    std::string configFile;          //-- Name of the configFile that the resource Finder will seek.
    
    yarp::os::Port handlerPort;      //-- A port to handle messages.
    
    //-- Pointer to a new thread to be created and 
    //-- started in configure() and stopped in close().
    tutorialPeriodicThread *pThread; 


  public:

    /* ===========================================================================
     *  Configure all the parameters and return true if successful.
     * 
     * @param rf : Reference to the resource finder.
     * 
     * @return Flag for the success.
     * =========================================================================== */
    bool configure(yarp::os::ResourceFinder &rf); 

   
    /* ===========================================================================
     *  Interrupt, e.g., the ports. 
     * =========================================================================== */
    bool interruptModule();                    


    /* ===========================================================================
     *  Close and shut down the tutorial.
     * =========================================================================== */
    bool close();


    /* ===========================================================================
     *  To respond through rpc port.
     * 
     * @param command : Command reference to bottle given to rpc port of module, alongwith parameters.
     * @param reply   : Reply reference to bottle returned by the rpc port in response to command.
     * 
     * @return Bool flag for the success of response else termination of module.
     * =========================================================================== */
    bool respond(const yarp::os::Bottle& command, yarp::os::Bottle& reply);


    /* ===========================================================================
     *  Unimplemented.
     * =========================================================================== */
    double getPeriod();


    /* ===========================================================================
     *  Unimplemented
     * =========================================================================== */ 
    bool updateModule();
};

#endif // _TUTORIAL_MODULE_H__

//----- end-of-file --- ( next line intentionally left blank ) ------------------
