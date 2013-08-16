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
