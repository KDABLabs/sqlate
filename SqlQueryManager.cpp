/*
    Copyright (C) 2011-2013 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Andras Mantia <andras.mantia@kdab.com>

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

#include "SqlQueryManager.h"
#include "SqlQuery.h"
#include "SqlMonitor.h"
#include "SqlQueryCache.h"

#include <QApplication>
#include <QMap>
#include <QMessageBox>
#include <QDebug>
#include <QSqlDriver>
#include <QThread>
#include <QVariant>
#include <QMutex>

Q_GLOBAL_STATIC(QMutex, queryMutex)
Q_GLOBAL_STATIC(QMutex, monitorMutex)

SqlQueryManager* SqlQueryManager::s_instance = 0;

SqlQueryManager* SqlQueryManager::instance()
{
  if (!s_instance)
      s_instance = new SqlQueryManager();
  return s_instance;
}


SqlQueryManager::SqlQueryManager()
{
}

SqlQueryManager::~SqlQueryManager()
{
    s_instance = 0;
}

void SqlQueryManager::registerQuery(SqlQuery* query)
{
    QMutexLocker locker( queryMutex() );
    m_queries.append(query);
//Debug line that helps finding leaking queries. It is intentionally not a SQLDEBUG.
//     qDebug() << "Registered query: " << query->lastQuery() << query;
}

void SqlQueryManager::unregisterQuery(SqlQuery* query)
{
    QMutexLocker locker( queryMutex() );
    m_queries.removeAll(query);
//Debug line that helps finding leaking queries. It is intentionally not a SQLDEBUG.
//     qDebug() << "Unregistered query: " << query->lastQuery() << query;
}

void SqlQueryManager::registerMonitor(SqlMonitor* monitor)
{
    QMutexLocker locker( monitorMutex() );
    m_monitors.append(monitor);
}

void SqlQueryManager::unregisterMonitor(SqlMonitor* monitor)
{
    QMutexLocker locker( monitorMutex() );
    m_monitors.removeAll(monitor);
}

int SqlQueryManager::queryCount() const
{
   QMutexLocker locker( queryMutex() );
   return m_queries.size();
}

void SqlQueryManager::printQueries() const
{
    QMutexLocker locker( queryMutex() );
    //The debug lines are intentionally qDebug and not SQLDEBUG
    qDebug() << "Live queries: ";
    Q_FOREACH(SqlQuery *query, m_queries) {
        qDebug() << query->lastQuery();
    }
}

int SqlQueryManager::monitorCount() const
{
    QMutexLocker locker( monitorMutex() );
    return m_monitors.size();
}

void SqlQueryManager::printMonitors() const
{
    QMutexLocker locker( monitorMutex() );
    //The debug lines are intentionally qDebug and not SQLDEBUG
    qDebug() << "Active monitors: ";
    Q_FOREACH(SqlMonitor *monitor, m_monitors) {
        qDebug() << "Tables: " << monitor->monitoredTables() << " Values: " << monitor->monitoredValues();
    }
}


void SqlQueryManager::checkDbIsAlive(QSqlDatabase& db)
{
    QMutexLocker queryLocker( queryMutex() );
    QMutexLocker monitorLocker( monitorMutex() );

//     qDebug() << Q_FUNC_INFO << db.isOpen() << db.isValid() << ( !db.isOpen() || !db.isValid() );

    //store all the queries and their bound values
    QMap<SqlQuery*, QMap<QString, QVariant> > allBoundValues;
    Q_FOREACH(SqlQuery* q, m_queries) {
        if (q->connectionName() != db.connectionName() )
            continue;
        allBoundValues[q] = q->boundValues();
    }
    int retryCount = 0;
    while ( !db.isOpen() || !db.isValid() ) {
        if ( QThread::currentThread() == QCoreApplication::instance()->thread() ) {
#if 0
          if  ( QMessageBox::question( 0, QObject::tr("Database connection error"),
            QObject::tr("<qt>The database is not available.<br/><br/>Press <b>Retry</b> to try again or <b>Abort</b> to terminate the application.<br />\
            Unsaved changes are lost if you Abort.\
            <br/><br/>If the problem persist, please inform your system administrator.</qt>"), QMessageBox::Retry | QMessageBox::Abort, QMessageBox::Retry ) == QMessageBox::Abort ) {
              ::exit(1) ;
           }
#endif
            ::exit(1);
        }
        retryCount++;
        if (retryCount > 10 ) { //give up at some point
            return;
        }

       if ( db.open() ) {
        SqlQueryCache::clear(); // reconnected, so all cached queries are now invalid
        //if the connection was dropped and recreated all the prepared queries are invalid, so we need to reconstruct them
        //using the lastQuery() string and the saved bound values
        Q_FOREACH(SqlQuery* q, m_queries ) {
            if (q->connectionName() != db.connectionName() )
                continue;
            if ( !q->lastQuery().isEmpty() ) {
                QString str = q->lastQuery();
//                 qDebug() << "Prepare query again: " << q << str;
                q->prepareWithoutCheck( str ); //TODO: this generates a runtime warning as the old stored query (prepare) cannot be cleaned up as it got lost. I have no idea what to do.
                QMap<QString, QVariant> boundValues = allBoundValues[q];
                QMap<QString, QVariant>::const_iterator it;
                for (it = boundValues.constBegin(); it != boundValues.constEnd(); ++it) {
                    q->bindValue( it.key(), it.value() );
                }
            }
        }

        //unsubscribe from all the notifications. Important to do it here for all notifications of the db, as
        //the QPSQL implementation will not recreate the QSocketNotifier otherwise and we will not get any notifications
        QStringList notifications = db.driver()->subscribedToNotifications();
        Q_FOREACH(QString notification, notifications) {
            db.driver()->unsubscribeFromNotification(notification);
        }
        //subscribe again to the db notifications
        Q_FOREACH(SqlMonitor* monitor, m_monitors ) {
            if (monitor->database().connectionName() != db.connectionName() )
            continue;
           monitor->resubscribe();
        }
       }
    }
}




