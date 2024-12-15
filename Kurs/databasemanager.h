#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>

class DatabaseManager
{
public:
    DatabaseManager();
    static QSqlDatabase getDatabase();
    static bool initializeDatabase();

private:
    static QString dbPath;
};

#endif // DATABASEMANAGER_H
