/*
* Copyright (C) 2011-2013 Klaralavdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Author: Volker Krause <volker.krause@kdab.com>
*         Benoit Dumas <benoit.dumas@com>
*/
#ifndef SQLCREATERULE_H
#define SQLCREATERULE_H

#include "SqlInternals_p.h"
#include "SqlSchema_p.h"
#include "SqlQuery.h"
#include "SqlUtils.h"

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/placeholders.hpp>

#include <QStringList>

/**
 * @file SqlCreateRule.h
 * Classes and functions to create tables rules based on the schema definition in SqlSchema.h
 */

namespace Sql {

namespace detail {

/**
 * Table-wise notification rule creator.
 * @internal
 */
struct notification_rule_creator {
    notification_rule_creator( QStringList &stmts ) : m_stmts( stmts ) {}
    template <typename T>
    void operator()( wrap<T> ) {
        if ( T::is_lookup_table::value || T::is_relation::value )
            return;
        const QStringList ops = QStringList() << QLatin1String( "Insert" ) << QLatin1String( "Update" ) << QLatin1String( "Delete" );
        foreach ( const QString& op, ops ) {
            const QString stmt = QLatin1Literal( "CREATE OR REPLACE RULE " )
                            % T::tableName()
                            % QLatin1Literal( "Notification" ) % op % QLatin1Literal( "Rule AS ON " )
                            % op.toUpper() % QLatin1Literal( " TO " )
                            % T::tableName()
                            % QLatin1Literal( " DO ALSO NOTIFY " )
                            % T::tableName()
                            % QLatin1Literal( "Changed" );
            m_stmts.push_back( stmt );
        }
    }
    QStringList &m_stmts;
};

/**
 * Column-wise notification rule creator.
 * @internal
 */
struct column_notification_rule_creator {
    column_notification_rule_creator( QStringList &stmts ) : m_stmts( stmts ) {}
    template <typename T>
    void operator()( wrap<T> ) {
        if( T::notify::value ) //notify with the corresponding column content ( as the notify channel) when a row is modified
        {
            //be sure to have unique rule name and stay within the allowed name size. We don't care that it's cryptic as it isn't used anywhere. so use Uuids.
            QString ruleID = QUuid::createUuid().toString();
            ruleID.replace(QLatin1Char('{'), QLatin1Char('\"'));
            ruleID.replace(QLatin1Char('}'), QLatin1Char('\"'));

            const QString identifier = SqlUtils::createIdentifier(T::table::sqlName() % QLatin1Literal( "_" ) % T::sqlName());
            const QString stmt = QLatin1Literal( "CREATE OR REPLACE RULE " )
                    % ruleID % QLatin1Literal( " AS ON UPDATE TO " )
                    % T::table::sqlName()
                    % QLatin1Literal( " DO ALSO SELECT pg_notify(CAST (OLD." ) %  T::sqlName()
                    % QLatin1Literal( " AS text) || '_") % identifier % QLatin1Literal( "','')");
            m_stmts.push_back( stmt );
        }
    }
    QStringList &m_stmts;
};

/**
 * Table-wise notification rule creator (create statements for each Notify-enabled column in all tables).
 * @internal
 */
struct table_notification_rule_creator {
    table_notification_rule_creator( QStringList &stmts ) : m_stmts( stmts ) {}
    template <typename T>
    void operator()( wrap<T> ) {
        boost::mpl::for_each<typename T::columns, detail::wrap<boost::mpl::placeholders::_1> >( column_notification_rule_creator( m_stmts ) );
    }
    QStringList &m_stmts;
};

}

/**
 * Returns a list of CREATE RULE statements to create notification rules for all mutable tables.
 * @tparam Schema A MPL sequence of tables.
 * @returns A list of SQL statements to create notification rules for all mutable tables.
 */
template<typename Schema>
QStringList createNotificationRuleStatements()
{
    QStringList statements;
    boost::mpl::for_each<Schema, detail::wrap<boost::mpl::placeholders::_1> >( detail::notification_rule_creator( statements ) );
    boost::mpl::for_each<Schema, detail::wrap<boost::mpl::placeholders::_1> >( detail::table_notification_rule_creator( statements ) );

    return statements;
}

/**
 * Create all table change notification rules.
 * @tparam Schema A MPL sequence of tables.
 * @param db The database to create the tables in
 * @throws SqlException in case of a database error
 */
template <typename Schema>
void createNotificationRules( const QSqlDatabase &db )
{
    foreach ( const QString &stmt, Sql::createNotificationRuleStatements<Schema>() ) {
        SqlQuery q( db );
        q.exec( stmt );
    }
}

}

#endif
