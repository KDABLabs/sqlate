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

#ifndef SQL_MONITOR_H
#define SQL_MONITOR_H

#include <QObject>
#include <QSqlDatabase>
#include <QStringList>

#include "sqlate_export.h"
#include "SqlUtils.h"

#include <boost/preprocessor/repetition.hpp>

/**
 * Helper class for monitoring a set up tables for changes.
 */
class SQLATE_EXPORT SqlMonitor : public QObject
{
    Q_OBJECT
public:
    /**
     * Creates a table monitor for the given database.
     * @param db The database on which we want to monitor tables.
     * @param parent The parent object.
     */
    explicit SqlMonitor( const QSqlDatabase& db, QObject* parent = 0 );

    virtual ~SqlMonitor();

    /**
     * Set a list of table names to monitor.
     * @param tables List of table names.
     */
    void setMonitorTables( const QStringList &tables );


    /**
     * Monitor a row and emits a signal carrying the content of a selected column when an UPDATE is performed on this row.
     * @note you need to declare the column as "notification ready" in the SQL schema, using the flag "Notify"
     *
     * @param table the table in which we want the row to be monitored
     * @param column the monitored column
     *
     */
    template <typename T>
    bool addValueMonitor( const T&, const QVariant& value)
    {
        QString processedValue = value.toString();
        //remove braces in case of an uuid
        if(processedValue.contains(QLatin1Char('{')) && processedValue.contains(QLatin1Char('}')))
        {
            processedValue.remove(QLatin1Char('{'));
            processedValue.remove(QLatin1Char('}'));
        }

        const QString identifier = SqlUtils::createIdentifier(T::table::sqlName() + QLatin1Char( '_' ) + T::sqlName());
        const QString notification = QString::fromLatin1("%1_%2")
                .arg(processedValue)
                .arg(identifier);
        m_monitoredValues << notification; //maybe it's already registered in another instance of the monitor, just add it in the list in this case
        if(subscribe(notification))
        {
            return true;
        }
        else return false;
    }

    /**
     * @brief Subscribes again to the monitored notifications. Used after an unexpected database disconnection.
     **/
    void resubscribe();

    /**
     * @brief Unsubscribe from all "values" notifications
     **/
    void unsubscribeValuesNotifications();

    QSqlDatabase database() const { return m_db; }

    QStringList monitoredTables() const { return m_tables; }
    QStringList monitoredValues() const { return m_monitoredValues; }


#ifndef SQL_MONITOR_MAX_SIZE
#define SQL_MONITOR_MAX_SIZE 15
#endif

#define CONST_REF(z, n, unused) const T ## n &
#define TABLE_NAME(z, n, unused) << T ## n ::tableName()
#define MONITOR_IMPL(z, n, unused) \
    template <BOOST_PP_ENUM_PARAMS(n, typename T)> \
    void setMonitorTables( BOOST_PP_ENUM(n, CONST_REF, ~) ) { \
        setMonitorTables( QStringList() BOOST_PP_REPEAT(n, TABLE_NAME, ~) ); \
    }
BOOST_PP_REPEAT_FROM_TO(1, SQL_MONITOR_MAX_SIZE, MONITOR_IMPL, ~)

#undef MONITOR_IMPL
#undef CONST_REF
#undef TABLE_NAME


Q_SIGNALS:
    /**
     * Emitted when any of the monitored tables changed.
     */
    void tablesChanged();

    void notify( const QString &notification );

private Q_SLOTS:
    void notificationReceived( const QString &notification );

private:    
    bool subscribe( const QString& notification );

    QSqlDatabase m_db;
    QStringList m_tables;
    QStringList m_monitoredValues;
};

#endif
