/**
 * SQLite_DB.cpp
 */

/*************************************************************************************************************
 * INCLUDE
*************************************************************************************************************/
#include <unistd.h>
#include <fcntl.h>
#include "SQLite_DB.h"
#include "handler_error.h"
#include "Log_Process.h"

/*************************************************************************************************************
 * DEFINE
*************************************************************************************************************/
#define SQLITE_THREAD_ID                 "storage"

/*************************************************************************************************************
 * VARIABLES
*************************************************************************************************************/
static string FIFO_NAME = "logFifo";

/*************************************************************************************************************
 * PRIVATE FUNCTIONS
*************************************************************************************************************/

/*************************************************************************************************************
 * GLOBAL FUNCTIONS
*************************************************************************************************************/

SQLiteHandler::SQLiteHandler(const string& dbName)
{
    databaseName = dbName;
    db = NULL;
}

SQLiteHandler::~SQLiteHandler()
{
    closeDatabase();
}

bool SQLiteHandler::initialize()
{
    string logMessage;
    
    // Initialize a logger
    Log_Process Logger(FIFO_NAME);

    int rc = sqlite3_open(databaseName.c_str(), &db);
    if (rc) 
    {
        logMessage = "[" SQLITE_THREAD_ID "] Unable to connect to SQL server";
        Logger.write_log(logMessage);
        handle_error("sqlite3_open()");
    }

    logMessage = "[" SQLITE_THREAD_ID "] Connection to SQL server established";
    Logger.write_log(logMessage);

    const char* createTableSQL = 
        "CREATE TABLE IF NOT EXISTS SensorData ("
        "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "Measurement TEXT NOT NULL, "
        "Timestamp DATETIME DEFAULT CURRENT_TIMESTAMP"
        ");";

    char* errMsg = NULL;
    rc = sqlite3_exec(db, createTableSQL, NULL, NULL, &errMsg);
    if (rc != SQLITE_OK)
    {
        sqlite3_free(errMsg);
        handle_error("sqlite3_exec()");
    }

    logMessage = "[" SQLITE_THREAD_ID "] New table created";
    Logger.write_log(logMessage);

    return true;
}

// Insert a sensor measurement into the database
bool SQLiteHandler::insertMeasurement(const string& data)
{
    const char* insertSQL = "INSERT INTO SensorData (Measurement) VALUES (?);";
    sqlite3_stmt* stmt;

    int rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) 
    {
        handle_error("sqlite3_prepare_v2()");
    }

    sqlite3_bind_text(stmt, 1, data.c_str(), -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) 
    {
        sqlite3_finalize(stmt);
        handle_error("sqlite3_step");
    }

    sqlite3_finalize(stmt);
    return true;
}

// Close the database connection
void SQLiteHandler::closeDatabase() 
{
    if (db)
    {
        sqlite3_close(db);
        db = NULL;
    }
}
