/**
 * main.cpp
 */

/*************************************************************************************************************
 * INCLUDE
*************************************************************************************************************/
#include <iostream>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "Log_Process.h"

/*************************************************************************************************************
 * DEFINE
*************************************************************************************************************/
using namespace std;

#define handle_error(msg) \
    do { perror(msg); exit(EXIT_FAILURE); } while (0)


/*************************************************************************************************************
 * VARIABLES
*************************************************************************************************************/
static string FIFO_NAME = "logFifo";

static pthread_mutex_t fifo_lock = PTHREAD_MUTEX_INITIALIZER;

/*************************************************************************************************************
 * FUNCTIONS
*************************************************************************************************************/
/**
 * Data_Thread_Handler
 */
static void *Data_Thread_Handler(void *args) 
{
    int logfifoFd = open(FIFO_NAME.c_str(), O_WRONLY);

    if (logfifoFd == -1) 
    {
        handle_error("[Data_Thread] open()");
    }

    string logMessage = "Log message from Data Thread\n";

    while (1)
    {
        // cout << "Data Thread running ...\n";
        pthread_mutex_lock(&fifo_lock);
        write(logfifoFd, logMessage.c_str(), logMessage.size());
        pthread_mutex_unlock(&fifo_lock);
        sleep(2);
    }

    close(logfifoFd);
}

/**
 * Connection_Thread_Handler
 */
static void *Connection_Thread_Handler(void *args) 
{
    int logfifoFd = open(FIFO_NAME.c_str(), O_WRONLY);

    if (logfifoFd == -1) 
    {
        handle_error("[Connection_Thread] open()");
    }

    string logMessage = "Log message from Connection Thread\n";
    while (1)
    {
        // cout << "Connection Thread running ...\n";
        pthread_mutex_lock(&fifo_lock);
        write(logfifoFd, logMessage.c_str(), logMessage.size());
        pthread_mutex_unlock(&fifo_lock);

        sleep(2);
    }
}

/**
 * Storage_Thread_Handler
 */
static void *Storage_Thread_Handler(void *args) 
{
    int logfifoFd = open(FIFO_NAME.c_str(), O_WRONLY);

    if (logfifoFd == -1) 
    {
        handle_error("[Storage_Thread] open()");
    }

    string logMessage = "Log message from Storage Thread\n";
    while (1)
    {
        // cout << "Storage Thread running ...\n";
        pthread_mutex_lock(&fifo_lock);
        write(logfifoFd, logMessage.c_str(), logMessage.size());
        pthread_mutex_unlock(&fifo_lock);

        sleep(2);
    }
}

/**
 * main_process
 */
static int main_process(void)
{
    int ret;
    pthread_t Data_Thread;
    pthread_t Connection_Thread;
    pthread_t Storage_Thread;

    if (ret = pthread_create(&Data_Thread, NULL, &Data_Thread_Handler, NULL))
    {
        handle_error("pthread_create()");
    }
    if (ret = pthread_create(&Connection_Thread, NULL, &Connection_Thread_Handler, NULL))
    {
        handle_error("pthread_create()");
    }
    if (ret = pthread_create(&Storage_Thread, NULL, &Storage_Thread_Handler, NULL))
    {
        handle_error("pthread_create()");
    }

    pthread_join(Data_Thread, NULL);
    pthread_join(Connection_Thread, NULL);
    pthread_join(Storage_Thread, NULL);

    return 0;
}

int main(int argc, char *argv[])
{
    pid_t logger_pid;

    // Create FIFO for log process
    if (mkfifo(FIFO_NAME.c_str(), 0666) == -1)
    {
        if (errno != EEXIST)
        {
            handle_error("mkfifo()");
            exit(EXIT_FAILURE);
        }
    }

    logger_pid = fork();

    if (logger_pid == -1)
    {
        handle_error("fork()");
    }

    if (logger_pid == 0)
    {
        /* Log Process*/
        Log_Process Logger(FIFO_NAME);
        Logger.run();
    }
    else
    {
        /* Main Process */
        main_process();
    }

    // Remove FIFO
    unlink(FIFO_NAME.c_str());
    return 0;

}
