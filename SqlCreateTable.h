/*
* Copyright (C) 2011-2013 Klaralavdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Author: Volker Krause <volker.krause@kdab.com>
*         Andras Mantia <andras.mantia@kdab.com>
*/
#ifndef SQL_CREATETABLE_H
#define SQL_CREATETABLE_H

#include "sqlate_export.h"
#include "SqlInternals_p.h"
#include "SqlSchema_p.h"
#include "SqlQuery.h"
#include "SqlInsertQueryBuilder.h"
#include "SqlGrantPermission.h"
#include "SqlCreateRule.h"

#include <QDateTime>
#include <QTime>
#include <QStringBuilder>
#include <QStringList>
#include <QUuid>
#include <QVariant>
#include <QSqlDatabase>

#include <boost/bind.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/static_assert.hpp>

#include <algorithm>

/**
 * @file SqlCreateTable.h
 * Classes and functions to create tables based on the schema definition in SqlSchema.h
 */

namespace Sql {

template<typename T> QString createTableStatement();
template<typename T> QStringList createTableTriggerStatements();

namespace detail {

/** Type to string conversion. */
SQLATE_EXPORT QString sqlType( wrap<QString>, int size );
SQLATE_EXPORT QString sqlType( wrap<bool>, int size );
SQLATE_EXPORT QString sqlType( wrap<QUuid>, int size );
SQLATE_EXPORT QString sqlType( wrap<int>, int size );
SQLATE_EXPORT QString sqlType( wrap<QDateTime>, int size );
SQLATE_EXPORT QString sqlType( wrap<QTime>, int size );
SQLATE_EXPORT QString sqlType( wrap<QDate>, int size );
SQLATE_EXPORT QString sqlType( wrap<QByteArray>, int size );
SQLATE_EXPORT QString sqlType( wrap<float>, int size );

/**
 * Helper class to build a list of SQL names from tables or columns.
 * @internal
 */
struct sql_name_accumulator
{
    explicit sql_name_accumulator( QStringList &nameList ) : m_names( nameList ) {};
    template <typename T> void operator() ( wrap<T> )
    {
        m_names.push_back( T::sqlName() );
    }
    QStringList &m_names;
};

/**
 * Helper class to create a single column statement in a create table command.
 * @internal
 */
struct column_creator
{
    explicit column_creator( QStringList &c ) : cols( c ) {}

    /**
     * Creates the column statement.
     * @tparam C The column type
     */
    template<typename C> void operator()( wrap<C> )
    {
        QString colStmt = C::sqlName() % QLatin1Char( ' ' ) % sqlType( detail::wrap<typename C::type>(), C::size );
        if ( C::primaryKey::value ) {
            colStmt += QLatin1String( " PRIMARY KEY" );
        } else {
            if ( C::unique::value )
                colStmt += QLatin1String( " UNIQUE" );
            if ( C::notNull::value )
                colStmt += QLatin1String( " NOT NULL" );
        }
        if ( C::useDefault::value ) {
            // FIXME: this is a huge over-simplification
            // stringification of values depends on the type (e.g. quoting for strings)
            // also, we probably want defaults different than the values created by the default ctor
            const QVariant v = QVariant::fromValue<typename C::type>( typename C::type() );
            colStmt += QLatin1Literal( " DEFAULT " ) % v.toString();
        }
        colStmt += referenceStatement<C>( typename C::hasForeignKey() );
        cols.push_back( colStmt );
    }

    /**
     * Creates foreign key reference statement.
     * @tparam C The column type.
     */
    template<typename C> QString referenceStatement( boost::mpl::false_ ) { return QString(); }
    template<typename C> QString referenceStatement( boost::mpl::true_ )
    {
        QString refStmt = QLatin1Literal( " REFERENCES " )
            % C::referenced_column::table::tableName()
            % QLatin1Literal( " (" )
            % C::referenced_column::sqlName()
            % QLatin1Char( ')' );
        BOOST_MPL_ASSERT(( boost::mpl::not_<boost::mpl::and_<typename C::onDeleteCascade, typename C::onDeleteRestrict> > ));
        if ( C::onDeleteCascade::value )
            refStmt += QLatin1String(" ON DELETE CASCADE");
        else if ( C::onDeleteRestrict::value )
            refStmt += QLatin1String(" ON DELETE RESTRICT");
        return refStmt;
    }

    QStringList &cols;
};

/**
 * Helper class to create table constraint statements.
 * @internal
 */
struct table_constraint_creator
{
    explicit table_constraint_creator( QStringList &statements ) : m_statements( statements ) {};

    /** Create multi-column uniqueness constraints.
     * @tparam T MPL sequence of unique columns.
     * @todo Optional safety check: Check if all those columns are in the same table
     */
    template <typename T>
    void operator() ( UniqueConstraint<T> )
    {
        QStringList cols;
        sql_name_accumulator accu( cols );
        boost::mpl::for_each<T, detail::wrap<boost::mpl::placeholders::_1> >( accu );
        m_statements.push_back( QLatin1Literal( "UNIQUE( " ) % cols.join( QLatin1String( ", " ) ) % QLatin1Literal( " )" ) );
    }

    QStringList &m_statements;
};

/**
 * Helper class to create per column trigger statements, to disallow UPDATE on a column
 * @internal
 */
struct column_trigger_creator
{
    explicit column_trigger_creator( QStringList &t ) : triggers( t ) {}

    /**
     * Creates the triggers for columns
     * @tparam C The column type
     */
    template<typename C> void operator()( wrap<C> )
    {

        C::sqlName() % QLatin1Char( ' ' ) % sqlType( detail::wrap<typename C::type>(), C::size );
        if ( C::onUserUpdateRestrict::value || C::primaryKey::value ) {
            QString triggerName = C::name();
            triggerName.replace( QLatin1Char('.'), QLatin1Char('_') );
            QString stmt = QLatin1Literal("DROP TRIGGER IF EXISTS no_column_update_") % triggerName %
                           QLatin1Literal(" on ") % C::table::sqlName();
            triggers.push_back( stmt );            
            stmt = QLatin1Literal("CREATE TRIGGER no_column_update_") % triggerName %
                              QLatin1Literal(" BEFORE UPDATE on ") % C::table::sqlName() %
                              QLatin1Literal(" FOR EACH ROW WHEN (OLD.") % C::sqlName() %
                              QLatin1Literal(" IS DISTINCT FROM NEW.") % C::sqlName() %
                              QLatin1Literal(" ) EXECUTE PROCEDURE is_administrator()");
            triggers.push_back( stmt );
        }
    }

    QStringList &triggers;
};


/**
 * Helper class to create table creation statements.
 * @internal
 */
struct table_creator
{
    explicit table_creator( QStringList & statements ) : m_statements( statements ) {};

    /**
     * Create the table creation statement for table @p T.
     * @tparam T The table type.
     */
    template <typename T>
    void operator() ( wrap<T> )
    {
        m_statements.push_back( Sql::createTableStatement<T>() );
    }

    QStringList &m_statements;
};

/**
 * Helper class to create table trigger statements.
 * @internal
 */
struct table_trigger_creator
{
    explicit table_trigger_creator( QStringList & statements ) : m_statements( statements ) {};

    /**
     * Create the table creation statement for table @p T.
     * @tparam T The table type.
     */
    template <typename T>
    void operator() ( wrap<T> )
    {
        m_statements.append( Sql::createTableTriggerStatements<T>() );
    }

    QStringList &m_statements;
};
} // detail


/**
 * Returns the CREATE TABLE SQL command for a given table.
 * @internal For unit testing only
 * @tparam T The table type.
 */
template <typename T>
QString createTableStatement()
{
    BOOST_STATIC_ASSERT((hasAdminGroup<T>::value));
    BOOST_STATIC_ASSERT((hasUserGroup<T>::value));
    QStringList cols;
    detail::column_creator accu( cols );
    boost::mpl::for_each<typename T::columns, detail::wrap<boost::mpl::placeholders::_1> >( accu );

    detail::table_constraint_creator accu2( cols );
    boost::mpl::for_each<typename T::constraints>( accu2 );

    return QLatin1Literal( "CREATE TABLE " ) % T::tableName() % QLatin1Literal( " (\n" ) % cols.join( QLatin1String( ",\n" ) ) % QLatin1Literal( "\n)" );
}

template <typename T>
QString createTableStatement( const T & ) { return createTableStatement<T>(); }

/**
 * Returns a list of CREATE TABLE statements for a list of tables.
 * @internal For unit testing only
 * @tparam Tables A MPL sequence of tables.
 */
template <typename Tables>
QStringList createTableStatements()
{
    QStringList tabs;
    detail::table_creator accu( tabs );
    boost::mpl::for_each<Tables, detail::wrap<boost::mpl::placeholders::_1> >( accu );
    return accu.m_statements;
}


/**
 * Create the trigger statements for a single table.
 */
template <typename T>
QStringList createTableTriggerStatements()
{
    QStringList triggers;
    detail::column_trigger_creator accu( triggers );
    boost::mpl::for_each<typename T::columns, detail::wrap<boost::mpl::placeholders::_1> >( accu );

    return triggers;
}

template <typename T>
QStringList createTableTriggerStatements( const T & ) { return createTableTriggerStatements<T>(); }

/**
 * Returns a list of DROP TRIGGER / CREATE TRIGGER statements for a list of tables.
 * @tparam Tables A MPL sequence of tables.
 */
template <typename Tables>
QStringList createTableTriggers()
{
    QStringList tabs;
    detail::table_trigger_creator accu( tabs );
    boost::mpl::for_each<Tables, detail::wrap<boost::mpl::placeholders::_1> >( accu );
    return accu.m_statements;
}

/**
 * Returns a list of table names in the given schema.
 * @tparam Tables A MPL sequence of tables.
 * @returns A list of table names in @p Tables
 */
template <typename Tables>
QStringList tableNames()
{
    QStringList tableNames;
    boost::mpl::for_each<Tables, detail::wrap<boost::mpl::placeholders::_1> >( detail::sql_name_accumulator( tableNames ) );
    return tableNames;
}

/**
 * Creates all rules, permissions and triggers for the tables in @p Tables in the database @p db
 * @tparam Tables A MPL squence of tables.
 * @param db The database to create the tables in.
 * @throws SqlException in case of a database error
 */
template <typename Tables>
void createRulesPermissionsAndTriggers( const QSqlDatabase &db = QSqlDatabase::database() )
{
    Sql::createNotificationRules<Tables>( db );
    Sql::grantPermissions<Tables>( db );
    foreach ( const QString &stmt, Sql::createTableTriggers<Tables>() ) {
        SqlQuery q( db );
        q.exec( stmt );
    }
}


/**
 * Create all tables in @p Tables in the database @p db.
 * @tparam Tables A MPL sequence of tables.
 * @param db The database to create the tables in
 * @throws SqlException in case of a database error
 */
template <typename Tables>
void createTables( const QSqlDatabase &db )
{
    foreach ( const QString &stmt, Sql::createTableStatements<Tables>() ) {
        SqlQuery q( db );
        q.exec( stmt );
    }
    
    Sql::createRulesPermissionsAndTriggers<Tables>( db );
}

/**
 * set the database version in the database
 * @param The verison table to set
 */
template <typename Table>
void setVersion( const Table& table, const QSqlDatabase &db , int version)
{
    SqlInsertQueryBuilder qb( db );
    qb.setTable( table );
    qb.addColumnValue( table.version, version );
    qb.exec();
}


/**
 * Creates all tables in @p Tables in the database @p db <b>if</b> a tables with the same name does not already exist.
 * @tparam Tables A MPL squence of tables.
 * @param db The database to create the tables in.
 * @throws SqlException in case of a database error
 */
template <typename Tables>
void createMissingTables( const QSqlDatabase &db = QSqlDatabase::database() )
{
    QStringList existingTables = db.tables();
    std::transform(existingTables.begin(), existingTables.end(), existingTables.begin(), boost::bind(&QString::toLower, _1));
    QStringList newTables = Sql::tableNames<Tables>();
    std::transform(newTables.begin(), newTables.end(), newTables.begin(), boost::bind(&QString::toLower, _1));
    const QStringList newTableStmts = Sql::createTableStatements<Tables>();
    Q_ASSERT( newTables.size() == newTableStmts.size() );
    for ( QStringList::const_iterator it = newTables.constBegin(), it2 = newTableStmts.constBegin(); it != newTables.constEnd(); ++it, ++it2 ) {
        if ( existingTables.contains( *it ) )
            continue;
        SqlQuery q( db );
        q.exec( *it2 );
    }
}

}

#endif
