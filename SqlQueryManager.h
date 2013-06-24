/*
* Copyright (C) 2011 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Author: Andras Mantia <andras.mantia@kdab.com>
*/

#ifndef SQLQUERYMANAGER_H
#define SQLQUERYMANAGER_H
#include "sqlate_export.h"

#include <QList>

class SqlMonitor;
class QSqlDatabase;
class SqlQuery;

/**
 * @brief Keeps track of queries and handles unexpected database disconnections.
 **/
class SQLATE_EXPORT SqlQueryManager
{

public:
    /**
     * @brief Get an instance to the singleton object.
     *
     * @return SqlQueryManager*
     **/
    static SqlQueryManager* instance();

    /**
     * @brief Check if the database connection is still alive. If it is not, show a warning and offer
     * the user to retry the SQL command or abort it. Abort aborts the whole application.
     * Note that using this function might NOT detect the disconnection all the time, sometimes
     * it detects only if there was a query run that failed. It is advised to call the function
     * before executing/preparing a query or running other SQL commands and run once more
     * if the command fails to test if the error was because the database connection is dropped.
     * When SqlQuery or SqlTransaction is used there is no need to call the method,
     * they do it themselves.
     **/
    void checkDbIsAlive( QSqlDatabase& db );

    void registerQuery(SqlQuery* query);
    void unregisterQuery(SqlQuery* query);

    void registerMonitor(SqlMonitor *monitor);
    void unregisterMonitor(SqlMonitor *monitor);

    int  queryCount() const;

    int monitorCount() const;

    void printQueries() const;

    void printMonitors() const;

    ~SqlQueryManager();

private:
    SqlQueryManager();

    static SqlQueryManager*  s_instance;
    QList<SqlQuery*> m_queries; ///<the list of stored queries
    QList<SqlMonitor*> m_monitors; ///<the list of stored notification monitor s
};

#endif 
