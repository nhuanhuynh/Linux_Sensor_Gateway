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
#include "Sensor_Data_Queue.h"
#include "SQLite_DB.h"

/*************************************************************************************************************
 * DEFINE
*************************************************************************************************************/
using namespace std;

#define MAIN_DATA_THREAD_ID                 "data"
#define MAIN_CONNECT_THREAD_ID              "connect"
#define MAIN_STORAGE_THREAD_ID              "storage"

#define MAIN_TCP_SERVER_BUFF_SIZE           256
#define MAIN_SENSOR_TOO_COLD_THRESHOLD      15
#define MAIN_SENSOR_TOO_HOT_THRESHOLD       40
#define MAIN_NUM_OF_CONSUMERS               2
/*************************************************************************************************************
 * VARIABLES
*************************************************************************************************************/
static string FIFO_NAME = "logFifo";
static int MAIN_TCP_PORT_NUMBER = 0;

// Shared Data structure
struct ThreadData 
{
    Sensor_Data_Queue* sharedDataQueue;
};

/*************************************************************************************************************
 * FUNCTIONS
*************************************************************************************************************/

static void *Data_Thread_Handler(void *args) 
{
    string logMessage;
    string sensorData;

    // Get shared data queue argument
    ThreadData* data = (ThreadData *)args;
    Sensor_Data_Queue* sharedQueue = data->sharedDataQueue;

    // Initialize a logger
    Log_Process Logger(FIFO_NAME);

    while (1)
    {
        while (sharedQueue->pop(sensorData)) 
        {
            string nodeID_temp, nodeID;
            string temperature_node_temp, temperature_node;

            // Parse the command
            istringstream stream(sensorData);
            // Split string command
            stream >> nodeID_temp >> nodeID >> temperature_node_temp >> temperature_node;
            if (nodeID.empty())
            {
                logMessage = "[" MAIN_DATA_THREAD_ID "] Received sensor data with invalid sensor node ID";
                Logger.write_log(logMessage);
            }
            else if (stoi(temperature_node) < MAIN_SENSOR_TOO_COLD_THRESHOLD)
            {
                logMessage = "[" MAIN_DATA_THREAD_ID "] The sensor node with nodeID: " + nodeID + 
                             " reports it is too cold (running avg temperature = " + temperature_node + ")";
                Logger.write_log(logMessage);
            }
            else if (stoi(temperature_node) > MAIN_SENSOR_TOO_HOT_THRESHOLD)
            {
                logMessage = "[" MAIN_DATA_THREAD_ID "] The sensor node with nodeID: " + nodeID + 
                             " reports it is too hot (running avg temperature = " + temperature_node + ")";
                Logger.write_log(logMessage);
            }
        }
    }
    return NULL;
}

static void *Connection_Thread_Handler(void *args) 
{
    // Initialize TCP socket server, creat a subject for it
    int server_fd, new_socket_fd;
    int opt;
    int len;
    struct sockaddr_in serv_addr, client_addr;
    string logMessage;
    int numb_read;
    char recvbuff[MAIN_TCP_SERVER_BUFF_SIZE];
    string nodeID;

    // Get shared data queue argument
    ThreadData* data = (ThreadData *)args;
    Sensor_Data_Queue* sharedQueue = data->sharedDataQueue;

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

    // Initialize a logger
    Log_Process Logger(FIFO_NAME);

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

        while (1)
        {
            memset(recvbuff, 0, sizeof(recvbuff));
            // Read data from client
            numb_read = read(new_socket_fd, recvbuff, sizeof(recvbuff)-1);
            if(numb_read == -1)
            {
                handle_error("read()");
            }
            if (numb_read == 0)
            {
                logMessage = "[" MAIN_CONNECT_THREAD_ID "] The sensor node with ID: " + nodeID + " has closed the connection";
                Logger.write_log(logMessage);
                break;
            }

            string temp_node, temperature_node;

            // Parse the command
            istringstream stream(recvbuff);
            // Split string command
            stream >> temp_node >> nodeID >> temperature_node;

            if (temperature_node == "temperature:")
            { // Receive temperature values
                string str(recvbuff);
                // Push to Queue
                sharedQueue->push(str);
                sharedQueue->set_done();
            }
            else
            {
                logMessage = "[" MAIN_CONNECT_THREAD_ID "] A sensor node with ID: " + nodeID + " has opened a new connection";
                Logger.write_log(logMessage);
            }
        }
    }
    return NULL;
}

static void *Storage_Thread_Handler(void *args) 
{
    string sensorData;
    string logMessage;

    // Get shared data queue argument
    ThreadData* data = (ThreadData *)args;
    Sensor_Data_Queue* sharedQueue = data->sharedDataQueue;

    // Initilize SQL DB
    SQLiteHandler sqlHandler("sensor_data.db");
    if (!sqlHandler.initialize()) 
    {
        handle_error("sqlHandler.initialize()");
    }

    // Initialize a logger
    Log_Process Logger(FIFO_NAME);

    while (1)
    {
        while (sharedQueue->pop(sensorData)) 
        {
            // Write log, @TODO: create a function to write log
            logMessage = "[" MAIN_STORAGE_THREAD_ID "] Stored: " + sensorData;
            Logger.write_log(logMessage);

            // Insert sensor data into DB
            if (!sqlHandler.insertMeasurement(sensorData)) 
            {
                cerr << "[" MAIN_STORAGE_THREAD_ID "] Failed to insert measurement: " << sensorData << endl;
            }
        }
    }

    return NULL;
}

static int main_process(void)
{
    int ret;
    pthread_t Data_Thread;
    pthread_t Connection_Thread;
    pthread_t Storage_Thread;

    // Initialize Shared Data Queue
    Sensor_Data_Queue sharedQueue(MAIN_NUM_OF_CONSUMERS);
    ThreadData threadData = 
    {
        .sharedDataQueue = &sharedQueue
    };

    if (ret = pthread_create(&Data_Thread, NULL, &Data_Thread_Handler, &threadData))
    {
        handle_error("pthread_create()");
    }
    if (ret = pthread_create(&Connection_Thread, NULL, &Connection_Thread_Handler, &threadData))
    {
        handle_error("pthread_create()");
    }
    if (ret = pthread_create(&Storage_Thread, NULL, &Storage_Thread_Handler, &threadData))
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
