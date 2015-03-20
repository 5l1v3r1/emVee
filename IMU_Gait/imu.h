//
//  imu.h
//  IMU_Gait
//
//  Created by William Chavez-Salinas on 2/21/15.
//  Copyright (c) 2015 William Chavez-Salinas. All rights reserved.
//

#ifndef IMU_Gait_imu_h
#define IMU_Gait_imu_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>     
#include <termios.h>
#include <iostream>
#include <time.h>

class IMU
{
    public:
        int * currentStep;
        int * oldStep;
    
        int * currentStepYAxis;
        int * oldStepYAxis;
    
        int * currentStepZAxis;
        int * oldStepZAxis;
    
        unsigned int oldStepsRecorded;
        unsigned int currentStepsRecorded;
    
        unsigned int oldStepsRecordedY;
        unsigned int currentStepsRecordedY;

        unsigned int oldStepsRecordedZ;
        unsigned int currentStepsRecordedZ;

        int USB;
    
        IMU();
        void convertToNumericalNumber(std::string *values);
        void turnSerialPortOff();
        void turnSerialPortOn();
        void checkForChange();
        void sendOne();
        void wait(int seconds);
        void matchingAlgorithm();
        void copyCurrentStepToOldStep();
        void clearCurrentStep();
        int * parseInput(char * input);
        int * readInput(int USB, bool dataFlag);
        void setParameters(int USB);
        void matching();
};

#endif
