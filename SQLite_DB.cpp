/**
 * SQLite_DB.cpp
 */

/*************************************************************************************************************
 * INCLUDE
*************************************************************************************************************/
#include <sqlite3.h>
#include "SQLite_DB.h"
#include "handler_error.h"

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
    int rc = sqlite3_open(databaseName.c_str(), &db);
    if (rc) 
    {
        handle_error("sqlite3_open()");
    }

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
