/**
 * Log_Process.cpp
 */

/*************************************************************************************************************
 * INCLUDE
*************************************************************************************************************/
#include <iostream>

#include "handler_error.h"
#include "Sensor_Data_Queue.h"
/*************************************************************************************************************
 * DEFINE
*************************************************************************************************************/

/*************************************************************************************************************
 * VARIABLES
*************************************************************************************************************/

/*************************************************************************************************************
 * PRIVATE FUNCTIONS
*************************************************************************************************************/

/*************************************************************************************************************
 * GLOBAL FUNCTIONS
*************************************************************************************************************/

void Sensor_Data_Queue::push(const int& data)
{
    pthread_mutex_lock(&mutex);
    dataQueue.push(data);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

bool Sensor_Data_Queue::pop(int& data)
{
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    if (!dataQueue.empty()) 
    {
        data = dataQueue.front();
        dataQueue.pop();
        pthread_mutex_unlock(&mutex);
        return true;
    }
    pthread_mutex_unlock(&mutex);
    return false;
}

void Sensor_Data_Queue::set_done()
{
    pthread_mutex_lock(&mutex);
    done = true;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

bool Sensor_Data_Queue::is_done()
{
    bool ret;
    pthread_mutex_lock(&mutex);
    ret = done && dataQueue.empty();
    pthread_mutex_unlock(&mutex);
    return ret;
}
