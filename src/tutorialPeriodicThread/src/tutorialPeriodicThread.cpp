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
 * @file  tutorialPeriodicThread.cpp
 * @brief Implementation of the eventDriven thread (see tutorialPeriodicThread.h).
 * =========================================================================== */

#include <iCub/tutorialPeriodicThread.h>

#define THPERIOD 0.1 // seconds.

tutorialPeriodicThread::tutorialPeriodicThread() : 
    PeriodicThread(THPERIOD) {

    robot = "icub";        
}


tutorialPeriodicThread::tutorialPeriodicThread(std::string _robot, std::string _configFile) : 
    PeriodicThread(THPERIOD) {

    robot = _robot;
    configFile = _configFile;
}


tutorialPeriodicThread::~tutorialPeriodicThread() {
    //-- Do nothing.
}


bool tutorialPeriodicThread::threadInit() {

    //-- Opening the port for direct input.
    if (!inputPort.open(getName("/image:i").c_str())) {
        yError("Unable to open port to receive input.");
        return false;  //-- Unable to open. Let RFModule know so that it won't run.
    }

    if (!outputPort.open(getName("/img:o").c_str())) {
        yError("Unable to open port to send unmasked events.");
        return false;  //-- Unable to open. Let RFModule know so that it won't run.
    }

    yInfo("Initialization of the processing thread correctly ended.");

    return true;
}


void tutorialPeriodicThread::threadRelease() {

    //-- Stop all threads.
    inputPort.interrupt();
    outputPort.interrupt();

    //-- Close the threads.
    inputPort.close();
    outputPort.close();
}


void tutorialPeriodicThread::setName(std::string str) {
    this->name=str;
}


std::string tutorialPeriodicThread::getName(const char* p) {
    std::string str(name);
    str.append(p);
    return str;
}


void tutorialPeriodicThread::setInputPortName(std::string InpPort) {
    //-- Do nothing.
}


void tutorialPeriodicThread::run() {    
    
    if (inputPort.getInputCount()) {
        inputImage = inputPort.read(true);   //-- Blocking reading for synchr with the input
        result = processing();
    }

    if (outputPort.getOutputCount()) {
        *outputImage = outputPort.prepare();
        outputImage->resize(inputImage->width(), inputImage->height());
        outputPort.write();
    }

}


bool tutorialPeriodicThread::processing(){
    // here goes the processing...
    return true;
}


