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
#include <sstream>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "handler_error.h"
#include "Log_Process.h"

/*************************************************************************************************************
 * DEFINE
*************************************************************************************************************/
using namespace std;

#define MAIN_DATA_THREAD_ID             "data"
#define MAIN_CONNECT_THREAD_ID          "connect"
#define MAIN_STORAGE_THREAD_ID          "storage"

/*************************************************************************************************************
 * VARIABLES
*************************************************************************************************************/
static string FIFO_NAME = "logFifo";
static int MAIN_TCP_PORT_NUMBER = 0;

/*************************************************************************************************************
 * FUNCTIONS
*************************************************************************************************************/

static void *Data_Thread_Handler(void *args) 
{
    // pthread_t thread_id = pthread_self();
    // ostringstream thread_id_stream;

    // Convert thread_id to string
    // thread_id_stream << thread_id;

    int logfifoFd = open(FIFO_NAME.c_str(), O_WRONLY);

    if (logfifoFd == -1) 
    {
        handle_error("[Data_Thread] open()");
    }

    string logMessage = "[" MAIN_DATA_THREAD_ID "] Temperature: 27";

    while (1)
    {
        // cout << "Data Thread running ...\n";
        // pthread_mutex_lock(&Log_Process::fifo_lock);
        // write(logfifoFd, logMessage.c_str(), logMessage.size());
        // pthread_mutex_unlock(&Log_Process::fifo_lock);
        sleep(1);
    }

    close(logfifoFd);
}

static void *Connection_Thread_Handler(void *args) 
{
    // Initialize TCP socket server, creat a subject for it
    int server_fd, new_socket_fd;
    int opt;
    int len;
    struct sockaddr_in serv_addr, client_addr;
    int logfifoFd;
    string logMessage;

    // Socket create
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
        handle_error("socket()");

    // Prevent: “address already in use”
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
        handle_error("setsockopt()");

    // Server initial
    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(MAIN_TCP_PORT_NUMBER);
    serv_addr.sin_addr.s_addr =  INADDR_ANY;

    // Binding socket with server address
    if (bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1)
    {
        handle_error("bind()");
    }

    // Listen to 5 connection
    if (listen(server_fd, 50) == -1)
    {
        handle_error("listen()");
    }

    // Get client's information
    len = sizeof(client_addr);

    logfifoFd = open(FIFO_NAME.c_str(), O_WRONLY);

    if (logfifoFd == -1) 
    {
        handle_error("[Connection_Thread] open()");
    }

    while (1)
    {
        // listening for connection
        new_socket_fd  = accept(server_fd, (struct sockaddr*)&client_addr, (socklen_t *)&len);
        if (new_socket_fd == -1)
        {
            handle_error("accept()");
        }

        // Get client's IP
        string client_ip(INET_ADDRSTRLEN, '\0');
        inet_ntop(AF_INET, &client_addr.sin_addr, &client_ip[0], INET_ADDRSTRLEN);

        logMessage = "[" MAIN_CONNECT_THREAD_ID "] Accepted a new connection from address: " + client_ip + ", set up at port: " + to_string(ntohs(serv_addr.sin_port));
        pthread_mutex_lock(&Log_Process::fifo_lock);
        write(logfifoFd, logMessage.c_str(), logMessage.size());
        pthread_mutex_unlock(&Log_Process::fifo_lock);
    }
}

static void *Storage_Thread_Handler(void *args) 
{
    int logfifoFd = open(FIFO_NAME.c_str(), O_WRONLY);

    if (logfifoFd == -1) 
    {
        handle_error("[Storage_Thread] open()");
    }

    string logMessage = "[" MAIN_STORAGE_THREAD_ID "] Data saving...";
    while (1)
    {
        // pthread_mutex_lock(&Log_Process::fifo_lock);
        // write(logfifoFd, logMessage.c_str(), logMessage.size());
        // pthread_mutex_unlock(&Log_Process::fifo_lock);
        sleep(2);
    }
}

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

/*************************************************************************************************************
 * ENTRY FUNCTION
*************************************************************************************************************/
int main(int argc, char *argv[])
{
    pid_t logger_pid;

    // Get command line arguments
    if (argc < 2) 
    {
        handle_error("No port provided\ncommand: ./Sensor_Gateway <port number>\n");
    } 
    else
    {
        MAIN_TCP_PORT_NUMBER = atoi(argv[1]);
    }

    // Create FIFO for log process
    if (mkfifo(FIFO_NAME.c_str(), 0665) == -1)
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
