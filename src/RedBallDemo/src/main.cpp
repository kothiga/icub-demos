// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Author: Austin Kothig
 * email: kothiga2112@gmail.com
 * @file main.cpp
 * @brief main code for the red ball tracking module.
 */

#include <iCub/redBallDemoModule.h>

using namespace yarp::os;
using namespace yarp::sig;

int main(int argc, char* argv[]) {

    Network yarp;
    redBallDemoModule module;

    ResourceFinder rf;
    rf.setVerbose(true);
    //rf.setDefaultConfigFile(".ini");     //overridden by --from parameter
    //rf.setDefaultContext("HeadTurn");    //overridden by --context parameter
    rf.configure(argc, argv);

    module.runModule(rf);
    return 0;
}
