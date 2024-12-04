/**
 * Log_Process.cpp
 */

/*************************************************************************************************************
 * INCLUDE
*************************************************************************************************************/
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sstream>
#include <chrono>
#include <ctime>

#include "Log_Process.h"
#include "handler_error.h"
/*************************************************************************************************************
 * DEFINE
*************************************************************************************************************/
#define LOGGER_READ_BUFFER_LENGTH           256

/*************************************************************************************************************
 * VARIABLES
*************************************************************************************************************/

pthread_mutex_t Log_Process::fifo_lock = PTHREAD_MUTEX_INITIALIZER;

/*************************************************************************************************************
 * PRIVATE FUNCTIONS
*************************************************************************************************************/

string getCurrentTimestamp()
{
    // Get time
    auto now = chrono::system_clock::now();
    time_t now_time = chrono::system_clock::to_time_t(now);

    // Convert time data to string
    char buffer[100];
    if (strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&now_time)))
    {
        return string(buffer);
    }
    return "Unknown Time";
}

void Log_Process::writeToLogFile(vector<char> buffer, ssize_t length)
{
    ostringstream logStream;

    // Add timestamp
    logStream << "[" << getCurrentTimestamp() << "] ";
    // Convert FIFO message to string
    logStream.write(buffer.data(), length);
    // Add newline for new data
    logStream << '\n';

    cout << logStream.str();

    // Write log into gateway.log
    if(write(logFile_fd, logStream.str().c_str(), logStream.str().size()) == -1)
    {
        handle_error("[Log_Process] write()");
    }

    // Remove stream data
    logStream.str("");
    // Reset stream's status
    logStream.clear();
}

/*************************************************************************************************************
 * GLOBAL FUNCTIONS
*************************************************************************************************************/

Log_Process::Log_Process(string fifo)
{
    // Get FIFO name from Main Process
    fifoName = fifo;

    //Create gateway.log file
    logFile_fd = open("gateway.log", O_RDWR | O_CREAT | O_APPEND, 0665);
    if (logFile_fd == -1)
    {
        handle_error("[Log_Process] open()");
    }

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
        pthread_mutex_lock(&Log_Process::fifo_lock);
        ssize_t bytesRead = read(fifoFd, buffer.data(), buffer.size());
        pthread_mutex_unlock(&Log_Process::fifo_lock);

        if (bytesRead > 0) 
        {
            Log_Process::writeToLogFile(buffer, bytesRead);
        }
        else
        {
            handle_error("[Log_Process] read()");
            break;
        }
    }
    close(logFile_fd);
    return;
}

