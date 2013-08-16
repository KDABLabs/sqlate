/*
    Copyright (C) 2011-2013 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Volker Krause <volker.krause@kdab.com>
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

#ifndef SQLSELECTQUERYBUILDER_H
#define SQLSELECTQUERYBUILDER_H
#include "SqlConditionalQueryBuilderBase.h"
#include "SqlCondition.h"
#include "sqlate_export.h"

#include <QStringList>

#include <boost/mpl/assert.hpp>
#include <boost/mpl/or.hpp>
#include <boost/type_traits/is_same.hpp>

/** API to create SELECT queries in a safe way from code, without the risk of
 *  introducing SQL injection vulnerabilities or typos.
 */
class SQLATE_EXPORT SqlSelectQueryBuilder : public SqlConditionalQueryBuilderBase
{
public:
    enum JoinType {
        InnerJoin,
        LeftOuterJoin,
        RightOuterJoin,
        FullOuterJoin,
        CrossJoin
    };

    enum LockMode {
        NoLock,
        Lock,
        TryLock
    };

    enum UnionType {
        Union,
        UnionAll
    };

    /// Create a new query builder for the given database
    explicit SqlSelectQueryBuilder( const QSqlDatabase &db = QSqlDatabase::database() );

    /// Use SELECT DISTINCT
    void setDistinct( const bool flag=true ) { m_distinct = flag; }

    /// Use SELECT DISTINCT ON(...), PostgreSQL-only
    /// mutually exclusive with setDistinct()
    void setDistinctOn( const QString &distinctExpr ) { m_distinctOn = distinctExpr; }

    /// Get the current distinct query status
    bool distinct() { return m_distinct; }

    /// SELECT @p columnName AS @p label, ...
    void addColumn( const QString &columnName, const QString &label = QString() );
    template <typename Column>
    inline void addColumn( const Column &column, const QString &label = QString() )
    {
        addColumn( column.name(), label );
    }
    // disambiguation overload for the above methods, taking this away finds QLatin1String usage, which indicates manual column names...
    inline void addColumn( const QLatin1String &column, const QString &label = QString() )
    {
        addColumn(QString(column), label );
    }

    /**
     * Adds an arbitrary expression as column to the result set.
     * Essentially the same as the above, but don't use this for selecting a single unmodified column.
     */
    inline void addColumnExpression( const QString &expression, const QString &label = QString() )
    {
        addColumn( expression, label );
    }

    /// SELECT * FROM ...
    void addAllColumns();

    /// SELECT currentDateTime() - server time see base class.
    void addCurrentTimeStampColumn();
    /**
     * Join a table to the query.
     * @param joinType The type of JOIN you want to add.
     * @param table The table to join.
     * @param condition the ON condition for this join.
     */
    void addJoin( JoinType joinType, const QString &table, const SqlCondition &condition );
    template <typename Table>
    inline void addJoin( JoinType joinType, const Table &table, const SqlCondition &condition )
    {
        addJoin( joinType, table.tableName(), condition );
    }

    /**
     * Join a table to the query.
     * This is a convenience method to create simple joins like e.g. 'LEFT JOIN t ON c1 = c2'.
     * @param joinType The type of JOIN you want to add.
     * @param table The table to join.
     * @param col1 The first column for the ON statement.
     * @param col2 The second column for the ON statement.
     */
    void addJoin( JoinType joinType, const QString &table, const QString &column1, const QString &column2 );
    template <typename Table, typename Column1, typename Column2>
    inline void addJoin( JoinType joinType, const Table &table, const Column1 &column1, const Column2 &column2 )
    {
        BOOST_MPL_ASSERT(( boost::mpl::or_<boost::is_same<Table, typename Column1::table>, boost::is_same<Table, typename Column2::table> > ));
        SqlCondition c;
        c.addColumnCondition( column1, SqlCondition::Equals, column2 ); // this also checks that the types of both columns are equal
        addJoin( joinType, table, c );
    }

    /// ... ORDER BY @p column @p sortOrder ...
    void addSortColumn( const QString& column, Qt::SortOrder sortOrder = Qt::AscendingOrder );
    template <typename Column>
    inline void addSortColumn( const Column &column, Qt::SortOrder sortOrder = Qt::AscendingOrder )
    {
        addSortColumn( column.name(), sortOrder );
    }

    /// ... remove ORDER BY @p column ...
    void removeSortColumn( const QString& column);
    template <typename Column>
    inline void removeSortColumn( const Column &column )
    {
        removeSortColumn( column.name() );
    }

    /// ... GROUP BY @p column ...
    void addGroupColumn( const QString& column );
    template <typename Column>
    inline void addGroupColumn( const Column & column )
    {
        addGroupColumn( column.name() );
    }

    /// Returns the created query object, when called first, the query object is assembled and prepared
    /*reimp*/ SqlQuery& query();

    /**
     * @brief Limit the query results
     *
     * @param offset starting offset
     * @param length max numbers of returned items
     **/
    void addLimit(uint offset, uint length);

public:
    /**
     * Select and add a exclusive row lock on the result set.
     * The query will block until existing row locks have been released.
     * @note The locks with stay active until the end of the transaction.
     * @tparam Table The table to lock rows on (needs to be in the sub-set of tables specified in FROM or JOIN)
     */
    template <typename Table>
    inline void lockExclusive( const Table &table )
    {
        m_lockTablesForUpdate.push_back(table.tableName());
        m_lockNoWait = false;
    }

    /**
     * Select and add a exclusive row lock on the result set.
     * The query will fail if other row locks exist on the result set.
     * @note The locks with stay active until the end of the transaction.
     * @tparam Table The table to lock rows on (needs to be in the sub-set of tables specified in FROM or JOIN)
     */
public:
    template <typename Table>
    inline void tryLockExclusive( const Table &table )
    {
        m_lockTablesForUpdate.push_back(table.tableName());
        m_lockNoWait = true;
    }

    /**
     * Create a composed query out of two. (see UNION keyword (SQL))
     * @param the queries to join together
     * @param the type of union
     */
    void combineQueries(SqlSelectQueryBuilder &query1, SqlSelectQueryBuilder &query2, const UnionType type = Union );

private:
    friend class SelectQueryBuilderTest;
    friend class SelectTest;

    /**
     * return the query as a formatted string
     */
    QString toString();

    /**
     * return the builder as a SqlQuery. The method throws an SqlException on error.
     */
    void prepareQuery();

    QVector<QVariant> bindValuesList();

    QVector<QPair<QString, QString> > m_columns;
    struct JoinInfo {
        JoinType type;
        QString table;
        SqlCondition condition;
    };
    QVector<JoinInfo> m_joins;
    QVector<QPair<QString, Qt::SortOrder> > m_sortColumns;
    QStringList m_groupColumns;
    QStringList m_lockTablesForUpdate;
    QString m_distinctOn;
    bool m_lockNoWait;
    bool m_distinct;
    uint m_limitOffset;
    uint m_limitLength;
};

#endif
