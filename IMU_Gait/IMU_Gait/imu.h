//
//  imu.h
//  IMU_Gait
//
//  Created by William Chavez-Salinas on 2/21/15.
//  Copyright (c) 2015 William Chavez-Salinas. All rights reserved.
//

#ifndef IMU_Gait_imu_h
#define IMU_Gait_imu_h

#include <stdio.h>      // standard input / output functions
#include <stdlib.h>
#include <string.h>     // string function definitions
#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>
#include <iostream>
#include <time.h>

void convertToNumericalNumber(std::string *values);
void setParameters(int USB);
void setOriginalGait(int * orig_Gait);
void compareGaits(int* orig_Gait, int* curr_Gait);
void write(int USB);
void matching();
int * parseInput(char * input);
void read(int USB, bool dataFlag);

#endif
