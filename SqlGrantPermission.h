/*
* Copyright (C) 2012-2013 Klaralavdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Author: Andras Mantia <andras.mantia@kdab.com>
*/
#ifndef SQLGRANTPERMISSION_H
#define SQLGRANTPERMISSION_H

#include "SqlInternals_p.h"
#include "SqlSchema_p.h"
#include "SqlQuery.h"

#include <boost/mpl/for_each.hpp>
#include <boost/mpl/placeholders.hpp>

#include <QStringList>

/**
 * @file SqlGrantPermission.h
 * Classes and functions to grant tables rights based on the schema definition in SqlSchema.h
 */

namespace Sql {

namespace detail {

/**
 * Permission statement creator.
 * @internal
 */
struct permission_statement_creator {
    permission_statement_creator( QStringList &stmts ) : m_stmts( stmts ) {}
    template <typename T>
    void operator()( wrap<T> ) {
        QString stmt = QLatin1Literal("GRANT SELECT, INSERT, UPDATE, DELETE ON ") % T::tableName() % QLatin1Literal(" TO GROUP sqladmins");
        m_stmts.push_back( stmt );
        QStringList allowedOperations;
        allowedOperations << QLatin1String("SELECT");
        if ( !T::is_restricted::value ) { 
            if ( T::delete_rows::value) {
                allowedOperations << QLatin1String("DELETE");
            }
            if ( T::update_rows::value) {
                allowedOperations << QLatin1String("UPDATE");
            }
            if ( T::insert_rows::value) {
                allowedOperations << QLatin1String("INSERT");
            }
        }
        stmt = QLatin1Literal("GRANT ") % allowedOperations.join(QLatin1String(", ")) %  QLatin1Literal(" ON ") % T::tableName() % QLatin1Literal(" TO GROUP sqlusers");
        m_stmts.push_back( stmt );

    }
    QStringList &m_stmts;
};

}

/**
 * Returns a list of GRANT ... ON ... TO ... statements for all mutable tables.
 * @tparam Schema A MPL sequence of tables.
 * @returns A list of SQL statements to give rights to the tables.
 */
template<typename Schema>
QStringList createPermissionStatements()
{
    QStringList statements;
    boost::mpl::for_each<Schema, detail::wrap<boost::mpl::placeholders::_1> >( detail::permission_statement_creator( statements ) );
    return statements;
}

/**
 * Set up permissings for all tables from a Schema.
 * @tparam Schema A MPL sequence of tables.
 * @param db The database to create the tables in
 * @throws SqlException in case of a database error
 */
template <typename Schema>
void grantPermissions( const QSqlDatabase &db )
{
    foreach ( const QString &stmt, Sql::createPermissionStatements<Schema>() ) {
        SqlQuery q( db );
        q.exec( stmt );
    }
}

}

#endif
