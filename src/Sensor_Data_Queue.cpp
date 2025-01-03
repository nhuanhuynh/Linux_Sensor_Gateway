/**
 * Sensor_Data_Queue.cpp
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
static int consumCounter = 0;
/*************************************************************************************************************
 * PRIVATE FUNCTIONS
*************************************************************************************************************/

/*************************************************************************************************************
 * GLOBAL FUNCTIONS
*************************************************************************************************************/

Sensor_Data_Queue::Sensor_Data_Queue(int num)
{
    Sensor_Data_Queue::numofConsumer = num;
}

void Sensor_Data_Queue::push(const string& data)
{
    pthread_mutex_lock(&mutex);
    dataQueue.push(data);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

bool Sensor_Data_Queue::pop(string& data)
{
    pthread_mutex_lock(&mutex);
    pthread_cond_wait(&cond, &mutex);
    if (!dataQueue.empty()) 
    {
        data = dataQueue.front();
        consumCounter--;
        if (consumCounter == 0)
        {
            dataQueue.pop();
            pthread_mutex_unlock(&mutex);
            return true;
        }
        pthread_mutex_unlock(&mutex);
        return true;
    }
    pthread_mutex_unlock(&mutex);
    return false;
}

void Sensor_Data_Queue::set_done()
{
    pthread_mutex_lock(&mutex);
    consumCounter = Sensor_Data_Queue::numofConsumer;
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
