/*
* Copyright (C) 2011 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Author: Andras Mantia <andras.mantia@kdab.com>
*/

#include "SqlMonitor.h"
#include "SqlQueryManager.h"


#include <QSqlDriver>
#include <QStringBuilder>

#include <QVariant>

SqlMonitor::SqlMonitor( const QSqlDatabase& db, QObject *parent ): QObject(parent), m_db(db)
{
    connect( db.driver(), SIGNAL(notification(QString)), SLOT(notificationReceived(QString)) );
    SqlQueryManager::instance()->registerMonitor(this);
}

SqlMonitor::~SqlMonitor()
{
    SqlQueryManager::instance()->unregisterMonitor(this);
}


void SqlMonitor::setMonitorTables( const QStringList& tables )
{
    foreach( const QString &table, tables ) {
        const QString notification = table.toLower() % QLatin1Literal( "changed" );
        if ( m_tables.contains( notification ) )
            continue;
        m_tables.push_back( notification );
        subscribe(notification);
    }
}

bool SqlMonitor::subscribe( const QString& notification )
{
    if ( m_db.driver()->subscribedToNotifications().contains( notification ) )
        return false;
    m_db.driver()->subscribeToNotification( notification );
    return true;
}

void SqlMonitor::notificationReceived( const QString& notification )
{
    if ( m_tables.contains(notification.toLower()) )
        emit tablesChanged();
    else if ( m_monitoredValues.contains(notification) )
        emit notify(notification);
}

void SqlMonitor::resubscribe()
{
    foreach( const QString &notification, m_tables ) {
        m_db.driver()->subscribeToNotification( notification );
    }
    foreach( const QString &notification, m_monitoredValues ) {
        m_db.driver()->subscribeToNotification( notification );
    }
}

void SqlMonitor::unsubscribeValuesNotifications()
{
    foreach( const QString &notification, m_monitoredValues ) {
        m_db.driver()->unsubscribeFromNotification( notification );
    }
    m_monitoredValues.clear();
}

#include "moc_SqlMonitor.cpp"
