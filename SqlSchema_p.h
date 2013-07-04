/*
* Copyright (C) 2011-2013 Klaralavdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Author: Volker Krause <volker.krause@kdab.com>
*         Andras Mantia <andras.mantia@kdab.com>
*/
#ifndef SQLSCHEMA_P_H
#define SQLSCHEMA_P_H

#include "SqlInternals_p.h"
#include "sqlate_export.h"
#include <QLatin1Literal>
#include <QUuid>

#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/comma_if.hpp>
#include <boost/preprocessor/seq.hpp>

/**
 * @file SqlSchema_p.h
 * Database schema related internal classes and functions.
 */


/** Stringification of SQL identifier names, used in table and column classes. */
#define SQL_NAME( x ) \
   static QString sqlName() { return QLatin1String(x); }

/** Define who can have admin rights for the table. Users belonging to the group specified here are treated as admin user.*/
#define ADMIN_GROUP( x ) \
   static QString adminGroup() { return QLatin1String(x); }

/** Define the group name for regular users valid for this table.*/
#define USER_GROUP( x ) \
   static QString userGroup() { return QLatin1String(x); }

/** Convenience macro to create column types.
 *  Use inside a table declaration.
 */
#define COLUMN( name, type, ... ) \
    struct name ## Type : Column< name ## Type, type , ## __VA_ARGS__ > { SQL_NAME( #name ) } name

/** Convenience macro to create a column alias.
  * Use inside a table definition to asign a different name to an existing column.
  */
#define COLUMN_ALIAS( column, alias ) \
    typedef column ## Type alias ## Type; \
    alias ## Type alias

/** Convenience macro for creating foreign key columns with the default legacy fk_[table]_[col] name.
 *  Use inside a table declaration.
 */
#define FOREIGN_KEY( name, fkTab, fkCol, ... ) \
    struct name ## Type : ForeignKey< name ## Type, fkTab ## Type :: fkCol ## Type , ## __VA_ARGS__ > { \
        static QString sqlName() { return QLatin1Literal( "fk_" ) % referenced_column::table::tableName() % QLatin1Char('_' ) % referenced_column::sqlName(); } \
    } name

/** Convenience macro for creating foreign key columns .
 *  Use inside a table declaration.
 */
#define NAMED_FOREIGN_KEY( name, fkTab, fkCol, ... ) \
    struct name ## Type : ForeignKey< name ## Type, fkTab ## Type :: fkCol ## Type , ## __VA_ARGS__ > { SQL_NAME( #name ) } name

/** Convenience macro to create a table with non-updatable rows type. Put after one of the TABLE macros.*/
#define NO_USER_UPDATE \
    typedef boost::mpl::false_ update_rows;

/** Convenience macro to create a table with non-deletable rows type. Put after one of the TABLE macros.*/
#define NO_USER_DELETE \
    typedef boost::mpl::false_ delete_rows;

/** Convenience macro to create a table with non-insertable rows type. Put after one of the TABLE macros.*/
#define NO_USER_INSERT \
    typedef boost::mpl::false_ insert_rows;

/** Convenience macro to create a restricted table type: only SELECT is allowed. Put after one of the TABLE macros.
 * Same as using NO_USER_UPDATE, NO_USER_DELETE, NO_USER_INSERT together
 */
#define ONLY_USER_SELECT \
    typedef boost::mpl::true_ is_restricted;

/** Convenience macro to create a table type. */
#define TABLE( name, _EXPORT ) \
    struct name ## Type; \
    extern _EXPORT name ## Type name; \
    struct name ## Type : Table< name ## Type >

/** Convenience macro to create a lookup table type. */
#define LOOKUP_TABLE( name, _EXPORT ) \
    struct name ## Type; \
    extern _EXPORT name ## Type name; \
    struct name ## Type : LookupTable< name ## Type >

/** Convenience macro to create a n:m helper table type. */
#define RELATION( name, leftTab, leftCol, rightTab, rightCol,_EXPORT ) \
    struct name ## Type; \
    extern _EXPORT name ## Type name; \
    struct name ## Type : Relation< name ## Type, leftTab ## Type :: leftCol ## Type, rightTab ## Type :: rightCol ## Type>

/** Convenience macro to create a n:m helper table type. */
#define UNIQUE_RELATION( name, leftTab, leftCol, rightTab, rightCol,_EXPORT ) \
    struct name ## Type; \
    extern _EXPORT name ## Type name; \
    struct name ## Type : UniqueRelation< name ## Type, leftTab ## Type :: leftCol ## Type, rightTab ## Type :: rightCol ## Type>

/** Convenience macro to create a recursive n:m helper table type. */
#define RECURSIVE_RELATION( name, tab, col, _EXPORT ) \
    struct name ## Type; \
    extern _EXPORT name ## Type name; \
    struct name ## Type : RecursiveRelation< name ## Type, tab ## Type :: col ## Type >

/** Convenience macro to create a recursive n:m helper table type. */
#define UNIQUE_RECURSIVE_RELATION( name, tab, col, _EXPORT ) \
    struct name ## Type; \
    extern _EXPORT name ## Type name; \
    struct name ## Type : UniqueRecursiveRelation< name ## Type, tab ## Type :: col ## Type >

/** Convenience macro to create the schema declaration. */
#define DECLARE_SCHEMA_MAKE_TYPE( r, unused, i, t ) BOOST_PP_COMMA_IF(i) BOOST_PP_CAT(t, Type)
#define DECLARE_SCHEMA( name, seq ) typedef boost::mpl::vector<BOOST_PP_SEQ_FOR_EACH_I(DECLARE_SCHEMA_MAKE_TYPE, ~, seq)> name

/** Convenience macro to create the schema definition. */
#define DEFINE_SCHEMA_MAKE_DEF( r, unused, t ) BOOST_PP_CAT(t, Type) t;
#define DEFINE_SCHEMA( seq ) BOOST_PP_SEQ_FOR_EACH(DEFINE_SCHEMA_MAKE_DEF, ~, seq)


namespace Sql {

/**
 * Properties of SQL columns.
 */
enum ColumnProperties
{
    Null = 0, ///< column can be NULL
    NotNull = 1, ///< column must not be NULL
    Unique = 2, ///< column content must be unique
    PrimaryKey = 4 | NotNull | Unique, ///< column is the primary key
    Default = 8, ///< Use default value
    OnDeleteCascade = 16, ///< For foreign key constraints, cascade on deletion.
    OnDeleteRestrict = 32, ///< for foreign key constraints, restrict deletion.
    OnUserUpdateRestrict = 64, ///< Restrict update for regular users. For this to work there must be an is_administrator() stored procedure that checks if the current user is administrator or not.
    Notify = 128 ///< When an UPDATE is made on a row, emit a signal containing the content of the column for this row, and an encoded string corresponding to the column name.
};


/**Type trait to check if a class has a certain method*/

#define HAS_MEMBER_METHOD(name, method) \
template <typename T>                   \
class name                              \
{                                       \
    struct Yes {char unused[1];};       \
    struct No {char unused[2];};        \
                                        \
    template <typename C>               \
    static Yes test(decltype(std::declval< const C>().method())*); \
    template <typename C>               \
    static No test(...);                \
public:                                 \
    static const bool value = (sizeof(test<T>(0)) == sizeof(Yes)); \
};

HAS_MEMBER_METHOD(hasAdminGroup, adminGroup);
HAS_MEMBER_METHOD(hasUserGroup, userGroup);

/**
 * Multi-column uniqeness table constraint.
 * @tparam ColList An MPL sequence of columns whose tuple needs to be unique table-wide
 */
template <typename ColList>
struct UniqueConstraint
{
    typedef ColList columns;
};


/**
 * Base class for SQL table types.
 * @tparam DerivedTable CRTP
 */
template<typename DerivedTable>
struct Table
{
    typedef boost::mpl::false_ is_lookup_table;
    typedef boost::mpl::false_ is_relation;
    typedef boost::mpl::false_ is_restricted; //regular users can only select it
    typedef boost::mpl::true_ delete_rows;    //regular users can delete rows from it. _is_restricted takes precendece
    typedef boost::mpl::true_ update_rows;    //regular users can update rows from it. _is_restricted takes precendece
    typedef boost::mpl::true_ insert_rows;    //regular users can insert rows from it. _is_restricted takes precendece

    /**
     * Base class for SQL columns of table @p DerivedTable
     * @tparam Derived CRTP
     * @tparam ColumnType The C++ data type of this column
     * @tparam P Column property flags
     * @tparam Size The maximum size of the column (-1 for unrestricted)
     */
    template <typename Derived, typename ColumnType, int P = Sql::Null, int Size = -1>
    struct Column
    {
        /** C++ type of this column. */
        typedef ColumnType type;
        /** The table this column is in. */
        typedef DerivedTable table;
        /** The maximum size of this column, -1 means no restrictions. */
        static const int size = Size;

        /** For identification of column classes in WhereExpr operators. */
        typedef boost::mpl::true_ is_column;

        typedef boost::mpl::false_ hasForeignKey;
        typedef typename boost::mpl::if_c<(P & NotNull) != 0, boost::mpl::true_, boost::mpl::false_>::type notNull;
        typedef typename boost::mpl::if_c<(P & Unique) != 0, boost::mpl::true_, boost::mpl::false_>::type unique;
        typedef typename boost::mpl::if_c<((P & PrimaryKey) == PrimaryKey), boost::mpl::true_, boost::mpl::false_>::type primaryKey;
        typedef typename boost::mpl::if_c<(P & Default) != 0, boost::mpl::true_, boost::mpl::false_>::type useDefault;
        typedef typename boost::mpl::if_c<(P & OnUserUpdateRestrict) != 0, boost::mpl::true_, boost::mpl::false_>::type onUserUpdateRestrict;
        typedef typename boost::mpl::if_c<(P & Notify) != 0, boost::mpl::true_, boost::mpl::false_>::type notify;

        /** Returns the fully qualified name of this column, ie. "tableName.columnName". */
        static QString name() { return DerivedTable::sqlName() % QLatin1Char('.') % Derived::sqlName(); }
    };

    /**
     * Base class for SQL foreign key columns of table @p DerivedTable.
     * @tparam Derived CRTP
     * @tparam ForeignColumn Type of the column this one is a reference on.
     * @tparam P Property flags.
     */
    template <typename Derived, typename ForeignColumn, int P = Sql::NotNull>
    struct ForeignKey : Column<Derived, typename ForeignColumn::type, P>
    {
        typedef ForeignColumn referenced_column;
        typedef boost::mpl::true_ hasForeignKey;

        typedef typename boost::mpl::if_c<(P & OnDeleteCascade) != 0, boost::mpl::true_, boost::mpl::false_>::type onDeleteCascade;
        typedef typename boost::mpl::if_c<(P & OnDeleteRestrict) != 0, boost::mpl::true_, boost::mpl::false_>::type onDeleteRestrict;
    };

    /** Returns the SQL identifier of this table. */
    static QString tableName() { return DerivedTable::sqlName(); }

    /** Sequence of table constraints. */
    typedef boost::mpl::vector<> constraints;
    typedef constraints baseConstraints;
};

/**
 * Base class for lookup tables
 * @tparam DerivedTable CRTP
 */
template<typename DerivedTable>
struct LookupTable : Table<DerivedTable>
{
    typedef boost::mpl::true_ is_lookup_table;
    typedef boost::mpl::true_ is_restricted;
    typedef Table<DerivedTable> base_type; // needed for MSVC to compile the follow lines...
    struct idType : base_type::template Column<idType, QUuid, Sql::PrimaryKey> { SQL_NAME( "id" ) } id;
    struct shortDescriptionType : base_type::template Column<shortDescriptionType, QString, Null, 128> { SQL_NAME( "short_desc" ) } shortDescription;
    struct descriptionType : base_type::template Column<descriptionType, QString, Null, 255> { SQL_NAME( "description" ) } description;
    typedef boost::mpl::vector<idType, shortDescriptionType, descriptionType> columns;
    typedef columns baseColumns;
};

/**
 * Base class for n:m relation tables
 * @tparam DerivedTable CRTP
 * @tparam LeftColumn Column type of the lhs of the mapping.
 * @tparam RightColumn Column type of the rhs of the mapping.
 */
template<typename DerivedTable, typename LeftColumn, typename RightColumn>
struct Relation : Table<DerivedTable>
{
    typedef boost::mpl::true_ is_relation;
    typedef Table<DerivedTable> base_type;
    struct leftType : base_type::template ForeignKey<leftType, LeftColumn>
    {
        static QString sqlName() { return QLatin1Literal( "fk_" ) % LeftColumn::table::sqlName() % QLatin1Char('_' ) % LeftColumn::sqlName(); }
    } left;
    struct rightType : base_type::template ForeignKey<rightType, RightColumn>
    {
        static QString sqlName() { return QLatin1Literal( "fk_" ) % RightColumn::table::sqlName() % QLatin1Char('_' ) % RightColumn::sqlName(); }
    } right;

    typedef boost::mpl::vector<leftType, rightType> columns;
    typedef columns baseColumns;
};

/**
 * Convenience class for unique n:m releation helper classes.
 * @see Relataion
 */
template <typename DerivedTable, typename LeftColumn, typename RightColumn>
struct UniqueRelation : Relation<DerivedTable, LeftColumn, RightColumn>
{
    typedef Relation<DerivedTable, LeftColumn, RightColumn> base_type; // needed for MSVC to compile the follow lines...
    typedef boost::mpl::vector< UniqueConstraint< typename base_type::columns > > constraints;
};

/**
 * Base class recursive n:m relation tables
 * Basically a Relation pointing to the same column with both sides of the mapping.
 * @tparam DerivedTable CRTP
 * @tparam ColumnT The column this mapping works on.
 */
template<typename DerivedTable, typename ColumnT>
struct RecursiveRelation : Table<DerivedTable>
{
    typedef boost::mpl::true_ is_relation;
    typedef Table<DerivedTable> base_type;
    struct leftType : base_type::template ForeignKey<leftType, ColumnT>
    {
        static QString sqlName() { return QLatin1Literal( "fk_" ) % ColumnT::table::sqlName() % QLatin1Char('_') % ColumnT::sqlName(); }
    } left;
    struct rightType : base_type::template ForeignKey<rightType, ColumnT>
    {
        static QString sqlName() { return QLatin1Literal( "fk_" ) % ColumnT::table::sqlName() % QLatin1Char('_') % ColumnT::sqlName() % QLatin1Literal("_link"); }
    } right;

    typedef boost::mpl::vector<leftType, rightType> columns;
    typedef columns baseColumns;
};

/**
 * Convenience class for unique recursive n:m releation helper classes.
 * @see RecursiveRelataion
 */
template <typename DerivedTable, typename ColumnT>
struct UniqueRecursiveRelation : RecursiveRelation<DerivedTable, ColumnT>
{
    typedef RecursiveRelation<DerivedTable, ColumnT> base_type;
    typedef boost::mpl::vector< UniqueConstraint< typename base_type::columns > > constraints;
};


}

#endif
