/**
 * Sensor_Data_Queue.h
 */
#ifndef _SENSOR_DATA_QUEUE_
#define _SENSOR_DATA_QUEUE_
/*************************************************************************************************************
 * INCLUDE
*************************************************************************************************************/
#include <queue>
#include <pthread.h>

/*************************************************************************************************************
 * DEFINE
*************************************************************************************************************/
using namespace std;

/*************************************************************************************************************
 * VARIABLES
*************************************************************************************************************/

/*************************************************************************************************************
 * FUNCTIONS
*************************************************************************************************************/
class Sensor_Data_Queue
{
private:
    queue<string> dataQueue;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    bool done = false;
    int numofConsumer = 0;
public:
    Sensor_Data_Queue(int num);
    void push(const string& data);
    bool pop(string& data);
    void set_done();
    bool is_done();
};

#endif // _SENSOR_DATA_QUEUE_
