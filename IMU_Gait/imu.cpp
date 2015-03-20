//
//  imu.cpp
//  IMU_Gait
//
//  Created by William Chavez-Salinas on 3/15/15.
//  Copyright (c) 2015 William Chavez-Salinas. All rights reserved.
//

#include "imu.h"
#include <stdio.h>      // standard input / output functions
#include <stdlib.h>
#include <string.h>     // string function definitions
#include <unistd.h>     // UNIX standard function definitions
#include <fcntl.h>      // File control definitions
#include <errno.h>      // Error number definitions
#include <termios.h>
#include <iostream>
#include <time.h>

using namespace std;

IMU::IMU()
{
    currentStep = new int[2000]();
    oldStep = new int[2000]();
    currentStepYAxis = new int[2000]();
    oldStepYAxis = new int[2000]();
    currentStepZAxis = new int[2000]();
    oldStepZAxis = new int[2000]();
    
    oldStepsRecorded = 0;
    currentStepsRecorded = 0;
    oldStepsRecordedY = 0;
    currentStepsRecordedY = 0;
    oldStepsRecordedZ = 0;
    currentStepsRecordedZ = 0;
    
    USB = 0;
    turnSerialPortOn();
    setParameters(IMU::USB);
}


void IMU::turnSerialPortOff()
{
    close(USB);
}

void IMU::turnSerialPortOn()
{
    USB = open("/dev/cu.usbserial-A6026OWM", O_RDWR| O_NOCTTY );
    if(USB == 0)
    {
        printf("Serial device is not working.");
    }
}

void IMU::checkForChange()
{
    int i = 0;
    int j = 1;
    for(; i < 2000; i++)
    {
        if(currentStep[i] == oldStep[i])
        {
            j++;
        }
    }
    
    if(j == 2000)
    {
        turnSerialPortOff();
        USB = 0;
    }
}

void IMU::sendOne()
{
    char one = '1';
    write(USB, &one, 1);
}

void IMU::wait(int seconds)
{
    unsigned long returnTime = time(0) + seconds;
    
    while(time(0) < returnTime) { }
}

void IMU::setParameters(int USB)
{
    struct termios tty;
    struct termios tty_old;
    memset (&tty, 0, sizeof tty);
    
    /* Error Handling */
    if ( tcgetattr ( USB, &tty ) != 0 )
    {
        std::cout << "Error " << errno << " from tcgetattr: " << strerror(errno);
    }
    
    /* Save old tty parameters */
    tty_old = tty;
    
    /* Set Baud Rate */
    cfsetospeed (&tty, (speed_t)B57600);
    cfsetispeed (&tty, (speed_t)B57600);
    
    /* Setting other Port Stuff */
    tty.c_cflag     &=  ~PARENB;            // Make 8n1
    tty.c_cflag     &=  ~CSTOPB;
    tty.c_cflag     &=  ~CSIZE;
    tty.c_cflag     |=  CS8;
    
    tty.c_cflag     &=  ~CRTSCTS;           // no flow control
    tty.c_cc[VMIN]   =  1;                  // read doesn't block
    tty.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
    tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines
    
    /* Make raw */
    cfmakeraw(&tty);
    
    /* Flush Port, then applies attributes */
    tcflush( USB, TCIFLUSH );
    if ( tcsetattr ( USB, TCSANOW, &tty ) != 0) {
        std::cout << "Error " << errno << " from tcsetattr";
    }
    
}

void IMU::matchingAlgorithm()
{
    int i;
    int numberOfMatchedSteps = 0;
    int stillNoMatch = 0;
    for(i = 0; i < 2000; i++)
    {
        if( (currentStep[i] == 1) && (oldStep[i] == 1))
        {
            numberOfMatchedSteps++;
        }
        else if(i > 0)
        {
            if((currentStep[i] == 1) && (oldStep[i - 1] == 1))
            {
                numberOfMatchedSteps++;
            }
        }
        else if (!stillNoMatch && i < 1999)
        {
            if((currentStep[i] == 1) && (oldStep[i + 1] == 1))
                numberOfMatchedSteps++;
        }
        else if( i > 1)
        {
            if((currentStep[i] == 1) && (oldStep[i - 2] == 1))
            {
                numberOfMatchedSteps++;
            }
        }
        else if (!stillNoMatch && i < 1998)
        {
            if((currentStep[i] == 1) && (oldStep[i + 2] == 1))
                numberOfMatchedSteps++;
        }
    }
    cout << "Number of Matched Points: " << numberOfMatchedSteps << endl;
    cout << "Number of Points in Current Gait: " << currentStepsRecorded << endl;
    cout << "Number of Points in Old Gait: " << oldStepsRecorded << endl;
    
    if(( numberOfMatchedSteps <= (currentStepsRecorded + 25) )
       && ( numberOfMatchedSteps >= (currentStepsRecorded - 25)))
    {
        cout << "match found!" << endl;
    }
    else if ((numberOfMatchedSteps > (currentStepsRecorded + 5))
             || (numberOfMatchedSteps < (currentStepsRecorded - 5)))
    {
        cout << "no match" << endl;
    }
}

void IMU::copyCurrentStepToOldStep()
{
    int i;
    for(i = 0; i < 2000; i++)
    {
        oldStep[i] = currentStep[i];
        oldStepYAxis[i] = currentStepYAxis[i];
        oldStepZAxis[i] = currentStepZAxis[i];
    }
    
    oldStepsRecorded = currentStepsRecorded;
    oldStepsRecordedY = currentStepsRecordedY;
    oldStepsRecordedZ = currentStepsRecordedZ;
}

void IMU::clearCurrentStep()
{
    int i = 0;
    for(i = 0; i < 2000; i++)
    {
        currentStep[i] = 0;
        currentStepYAxis[i] = 0;
        currentStepZAxis[i] = 0;
    }
}

int * IMU::parseInput(char * input)
{
    string str(input);
    string delimiter = ",";
    size_t pos = 0;
    string token;
    string *arr = new string[3]();
    int *intVals = new int[3]();
    int i = 0;
    
    while ((pos = str.find(delimiter)) != string::npos)
    {
        usleep(10);
        token = str.substr(0, pos);
        arr[i] = token;
        str.erase(0, pos + delimiter.length());
        i++;
    }
    
    arr[i] = str;
    delimiter = "-";
    i =0;
    arr[0].erase(0, 2);
    arr[1].erase(0, 3);
    arr[2].erase(0, 3);
    
    while(i < 3)
    {
        arr[i].erase(arr[i].begin(), std::find_if(arr[i].begin(), arr[i].end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
        
        if(!arr[i].empty())
        {
            if((pos = arr[i].find(delimiter)) != string::npos) {
                arr[i].erase(0, 1);
                intVals[i] = 1000 - stoi(arr[i]);
            } else {
                intVals[i] = 1000 + stoi(arr[i]);
            }
            //cout << intVals[i] << endl;
        }
        i++;
    }
    
    return intVals;
}

int * IMU::readInput(int USB, bool dataFlag)
{
    int n = 0,
    spot = 0;
    char buf = '\0';
    char response[1024];
    int * data;
    memset(response, '\0', sizeof response);
    
    do
    {
        n = read(USB, &buf, 1);
        sprintf( &response[spot], "%c", buf );
        spot += n;
    }
    while(buf != '\r' && buf != '\n' && n > 0);
    
    if (n < 0)
    {
        std::cout << "Error reading: " << strerror(errno);
    }
    else if (n == 0)
    {
        std::cout << "Read nothing!";
    }
    else
    {
        //std::cout << response;
        
        if(dataFlag)
        {
            int * data = parseInput(response);
            return data;
        }
    }
    return 0;
}