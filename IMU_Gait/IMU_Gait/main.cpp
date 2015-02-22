#include "imu.h"

using namespace std;

static int * currentStep = new int[2000]();
static int * oldStep = new int[2000]();
static unsigned int oldStepsRecorded = 0;
static unsigned int currentStepsRecorded = 0;

void setParameters(int USB)
{
    struct termios tty;
    struct termios tty_old;
    memset (&tty, 0, sizeof tty);
    
    /* Error Handling */
    if ( tcgetattr ( USB, &tty ) != 0 ) {
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

void matchingAlgorithm()
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

void copyCurrentStepToOldStep()
{
    int i;
    for(i = 0; i < 2000; i++)
    {
        oldStep[i] = currentStep[i];
    }
    oldStepsRecorded = currentStepsRecorded;
}

void clearCurrentStep()
{
    int i = 0;
    for(i = 0; i < 2000; i++)
    {
        currentStep[i] = 0;
    }
}

int * parseInput(char * input)
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

int * readInput(int USB, bool dataFlag)
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

int main(int argc, char** argv)
{
    /* open serial port and set parameters */
    int USB = open("/dev/cu.usbserial-A6026OWM", O_RDWR| O_NOCTTY );
    setParameters(USB);
    
    /* create the read set that you'll poll from */
    fd_set readset;
    FD_ZERO(&readset);
    
    /* input buffer for output from the IMU */
    char *input = (char*) malloc(sizeof(char) * 1024);
    int n;
    
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
            FD_SET(USB, &readset);
            FD_SET(0, &readset);
            tv.tv_sec = 1;
            tv.tv_usec = 0;
        
            if(select(USB + 1, &readset, NULL, NULL, &tv) < 0)
            {
                printf("error.\n");
                exit(0);
            }
            
            if (FD_ISSET(USB, &readset))
            {
                int * data = readInput(USB, dataBeingRead);
                
                if(recordedOldStep && data != 0)
                {
                    if(currentStep[data[0]] != 1)
                        currentStepsRecorded++;
                    currentStep[data[0]] = 1;
                    
                }
                if(!recordedOldStep && data != 0)
                {
                    if(oldStep[data[0]] != 1)
                        oldStepsRecorded++;
                    oldStep[data[0]] = 1;
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
            
                write(USB, input, n - 1);
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
            matchingAlgorithm();
            copyCurrentStepToOldStep();
            clearCurrentStep();
            currentStepsRecorded = 0;
        }
        
        recordedOldStep = true;
    }
}