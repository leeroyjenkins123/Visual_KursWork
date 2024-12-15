#include "databasemanager.h"

QString DatabaseManager::dbPath="/home/timofei/Coding/MyUniversityLabs/Third year/Visual programming and human-machine interaction Part 2/Kurs_Work/Test/test_database";

DatabaseManager::DatabaseManager()
{
    // Constructor implementation if needed
}

QSqlDatabase DatabaseManager::getDatabase()
{
    return QSqlDatabase::database();
}

bool DatabaseManager::initializeDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(dbPath);
    if (!db.open()) {
        // Handle error, e.g., display an error message
        return false;
    }
    return true;
}
