/*
    Copyright (C) 2011-2013 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Volker Krause <volker.krause@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/
#include "SqlQuery.h"
#include "SqlExceptions.h"
#include "SqlQueryManager.h"

#ifdef SQLATE_ENABLE_NETWORK_WATCHER
#include "SqlQueryWatcher.h"
#include "kdthreadrunner.h"
#endif

#include <QSqlDatabase>
#include <QDebug>
#include <QVariant>

SqlQuery::SqlQuery(const QString &query /*= QString()*/, const QSqlDatabase& db /*= QSqlDatabase()*/ ) :
    QSqlQuery( db ), m_db( db )
{
    m_connectionName = db.connectionName();

    SqlQueryManager::instance()->registerQuery(this);
    if ( !query.isEmpty() )
        exec( query );
}

SqlQuery::SqlQuery(const QSqlDatabase& db) :
    QSqlQuery( db ), m_db(db)
{
    m_connectionName = db.connectionName();
    SqlQueryManager::instance()->registerQuery(this);
}

SqlQuery::SqlQuery(const SqlQuery &other ) :
    QSqlQuery( other ), m_db(other.m_db)
{
    m_connectionName = m_db.connectionName();
    SqlQueryManager::instance()->registerQuery(this);
}

SqlQuery::~SqlQuery()
{
    SqlQueryManager::instance()->unregisterQuery(this);
}

SqlQuery& SqlQuery::operator=(const SqlQuery& other)
{
    QSqlQuery::operator=(other);
    m_db = other.m_db;
    m_connectionName = m_db.connectionName();
//Debug line that helps finding leaking queries. It is intentionally not a SQLDEBUG.
//     qDebug() << "operator= " <<  lastQuery() << this;
    return *this;
}

void SqlQuery::exec()
{
    SqlQueryManager::instance()->checkDbIsAlive(m_db);

#ifdef SQLATE_ENABLE_NETWORK_WATCHER
    KDThreadRunner<SqlQueryWatcherHelper> watcher;
    SqlQueryWatcherHelper* helper = watcher.startThread();
#endif

    bool result = QSqlQuery::exec();

#ifdef SQLATE_ENABLE_NETWORK_WATCHER
    QMetaObject::invokeMethod( helper, "quit", Qt::QueuedConnection );
    watcher.wait();
#endif

    if (!result && ( !m_db.isOpen() || !m_db.isValid() ) ) {
        SqlQueryManager::instance()->checkDbIsAlive(m_db);  //double check is needed, as Qt might not set m_db.isOpen() to false after connection loss if no queries were run meantime.
        result = QSqlQuery::exec();
    }

    if ( !result ) {
        qWarning() << Q_FUNC_INFO << "Exec failed: " << this << QSqlQuery::lastError() << " query was: " << QSqlQuery::lastQuery()
        << ", executed query: " << QSqlQuery::executedQuery() << " bound values: "<< QSqlQuery::boundValues().values();
//         qWarning() << "Database status: " << m_db.isOpen() << m_db.isValid() << m_db.isOpenError();
        throw SqlException( QSqlQuery::lastError() );
    }
}

void SqlQuery::exec(const QString& query)
{
    SqlQueryManager::instance()->checkDbIsAlive(m_db);

#ifdef SQLATE_ENABLE_NETWORK_WATCHER
    KDThreadRunner<SqlQueryWatcherHelper> watcher;
    SqlQueryWatcherHelper* helper = watcher.startThread();
#endif

    bool result = QSqlQuery::exec( query );
    if (!result && ( !m_db.isOpen() || !m_db.isValid() ) ) {
        SqlQueryManager::instance()->checkDbIsAlive(m_db);  //double check is needed, as Qt might not set m_db.isOpen() to false after connection loss if no queries were run meantime.
        result = QSqlQuery::exec();
    }

#ifdef SQLATE_ENABLE_NETWORK_WATCHER
    QMetaObject::invokeMethod( helper, "quit", Qt::QueuedConnection );
    watcher.wait();
#endif

    if ( !result ) {
        qWarning() << Q_FUNC_INFO << "Exec(query) failed: " << this << QSqlQuery::lastError() << " query was: " << QSqlQuery::lastQuery()
        << ", executed query: " << QSqlQuery::executedQuery()  << " bound values: "<< QSqlQuery::boundValues().values();
//         qWarning() << "Database status: " << m_db.isOpen() << m_db.isValid() << m_db.isOpenError();
        throw SqlException( QSqlQuery::lastError() );
    }
}

void SqlQuery::prepare(const QString& query)
{
    SqlQueryManager::instance()->checkDbIsAlive(m_db);

#ifdef SQLATE_ENABLE_NETWORK_WATCHER
    KDThreadRunner<SqlQueryWatcherHelper> watcher;
    SqlQueryWatcherHelper* helper = watcher.startThread();
#endif

    bool result = QSqlQuery::prepare( query );
    if (!result && ( !m_db.isOpen() || !m_db.isValid() ) ) {
        SqlQueryManager::instance()->checkDbIsAlive(m_db);  //double check is needed, as Qt might not set m_db.isOpen() to false after connection loss if no queries were run meantime.
        result = QSqlQuery::prepare( query );
    }

#ifdef SQLATE_ENABLE_NETWORK_WATCHER
    QMetaObject::invokeMethod( helper, "quit", Qt::QueuedConnection );
    watcher.wait();
#endif

    if ( !result ) {
        qWarning() << Q_FUNC_INFO << "Prepare failed: " << this << QSqlQuery::lastError() << " query was: " << QSqlQuery::lastQuery()
        << ", executed query: " << QSqlQuery::executedQuery()  << " bound values: "<< QSqlQuery::boundValues().values();
//         qWarning() << "Database status: " << m_db.isOpen() << m_db.isValid() << m_db.isOpenError();
        throw SqlException( QSqlQuery::lastError() );
    }
}

bool SqlQuery::prepareWithoutCheck(const QString& query)
{
    return QSqlQuery::prepare(query);
}
