/**
 * Log_Process.cpp
 */

/*************************************************************************************************************
 * INCLUDE
*************************************************************************************************************/
#include <iostream>
#include <string.h>
# include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <vector>
#include <sstream>

#include "Log_Process.h"
/*************************************************************************************************************
 * DEFINE
*************************************************************************************************************/
#define LOGGER_READ_BUFFER_LENGTH           256

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)

/*************************************************************************************************************
 * VARIABLES
*************************************************************************************************************/


/*************************************************************************************************************
 * FUNCTIONS
*************************************************************************************************************/

Log_Process::Log_Process(string fifo)
{
    fifoName = fifo;
    return;
}

void Log_Process::run(void)
{
    vector<char> buffer(LOGGER_READ_BUFFER_LENGTH);
    ostringstream logStream;


    int fifoFd = open(fifoName.c_str(), O_RDONLY);

    if (fifoFd == -1) 
    {
        handle_error("[Log_Process] open()");
    }

    while (1)
    {
        pthread_mutex_lock(&fifo_lock);
        ssize_t bytesRead = read(fifoFd, buffer.data(), buffer.size());
        pthread_mutex_unlock(&fifo_lock);

        if (bytesRead > 0) 
        {
            // Convert data to string
            logStream.write(buffer.data(), bytesRead);

            cout << "[Log_Process] Received: " << logStream.str();

            // Remove stream data
            logStream.str("");
            // Reset stream's status
            logStream.clear();
        }
        else
        {
            handle_error("[Log_Process] read()");
            break;
        }
    }

    return;
}

