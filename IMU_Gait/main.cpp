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

int main(int argc, char** argv)
{
    
    IMU *imu = new IMU();
    
    /* input buffer for output from the IMU */
    char *input = (char*) malloc(sizeof(char) * 1024);
    int n;
    
    fd_set readset;
    FD_ZERO(&readset);
    
    /* creating and setting the timer for the select statement */
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_sec = 0;
    
    bool dataBeingRead = false;
    bool recordedOldStep = false;
    bool recordedCurrentStep = false;
    
    while(1)
    {
        unsigned long returnTime = time(0) + 5;
        
        while(time(0) < returnTime)
        {
            FD_SET(imu->USB, &readset);
            FD_SET(0, &readset);
            tv.tv_sec = 1;
            tv.tv_usec = 0;
        
            if(select(imu->USB + 1, &readset, NULL, NULL, &tv) < 0)
            {
                printf("error.\n");
                exit(0);
            }
            
            if (FD_ISSET(imu->USB, &readset))
            {
                int * data = imu->readInput(imu->USB, dataBeingRead);
                
                if(recordedOldStep && data != 0)
                {
                    if(imu->currentStep[data[0]] != 1)
                        imu->currentStepsRecorded++;
                    imu->currentStep[data[0]] = 1;
                    
                }
                if(!recordedOldStep && data != 0)
                {
                    if(imu->oldStep[data[0]] != 1)
                        imu->oldStepsRecorded++;
                    imu->oldStep[data[0]] = 1;
                }
            }
            if (FD_ISSET(0, &readset))
            {
                fgets(input, 1024, stdin);
                n = strlen(input);
                cout << n << endl;
                
                if(n > 0 && input[n - 1] == '\n')
                {
                    input[n - 1] = '\0';
                }
                imu->sendOne();
                dataBeingRead = true;
            }
            bzero(input, 1024);
        }
        
        cout << "Step\n" << endl;
        
        if(recordedOldStep)
        {
            recordedCurrentStep = true;
        }
        
        if(recordedCurrentStep)
        {
            cout << "Comparing..." << endl;
            imu->matchingAlgorithm();
            imu->checkForChange();
            imu->copyCurrentStepToOldStep();
            imu->clearCurrentStep();
            imu->currentStepsRecorded = 0;
        }
        
        recordedOldStep = true;
        
        if(imu->USB == 0)
        {
            imu->turnSerialPortOn();
            imu->setParameters(imu->USB);
            FD_ZERO(&readset);
            input = (char*) malloc(sizeof(char) * 1024);
            tv.tv_sec = 1;
            tv.tv_sec = 0;
            dataBeingRead = false;
            recordedOldStep = false;
            recordedCurrentStep = false;
        }
    }
}