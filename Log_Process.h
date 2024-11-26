/**
 * Log_Process.h
 */
#ifndef _LOG_PROCESS_
#define _LOG_PROCESS_
/*************************************************************************************************************
 * INCLUDE
*************************************************************************************************************/

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
class Log_Process
{
private:
    string fifoName;
    pthread_mutex_t fifo_lock = PTHREAD_MUTEX_INITIALIZER;
public:
    Log_Process(string fifo);
    void run();
};

#endif // _LOG_PROCESS_
