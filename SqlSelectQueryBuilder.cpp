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

#include "SqlSelectQueryBuilder.h"

#include "SqlExceptions.h"
#include "SqlSchema.h"
#include "SqlGlobal.h"

SqlSelectQueryBuilder::SqlSelectQueryBuilder(const QSqlDatabase& db) :
    SqlConditionalQueryBuilderBase( db ),
    m_lockNoWait( false ),
    m_distinct( false ),
    m_limitOffset( -1 ),
    m_limitLength( -1 )
{
}

void SqlSelectQueryBuilder::addColumn(const QString& columnName, const QString& label)
{
    m_columns.push_back( qMakePair( columnName, label ) );
}

void SqlSelectQueryBuilder::addAllColumns()
{
    addColumn( QString::fromLatin1( "*" ) );
}

void SqlSelectQueryBuilder::addCurrentTimeStampColumn()
{
    addColumn( currentDateTime() );
}

void SqlSelectQueryBuilder::addJoin(SqlSelectQueryBuilder::JoinType joinType, const QString& table, const SqlCondition& condition)
{
    JoinInfo j;
    j.type = joinType;
    j.table = table;
    j.condition = condition;
    m_joins.push_back( j );
}

void SqlSelectQueryBuilder::addJoin(SqlSelectQueryBuilder::JoinType joinType, const QString& table, const QString& column1, const QString& column2)
{
    SqlCondition c;
    c.addColumnCondition( column1, SqlCondition::Equals, column2 );
    addJoin( joinType, table, c );
}

void SqlSelectQueryBuilder::addSortColumn(const QString& column, Qt::SortOrder sortOrder)
{
    m_sortColumns.push_back( qMakePair( column, sortOrder ) );
}

void SqlSelectQueryBuilder::removeSortColumn(const QString& column)
{
    QVector<QPair<QString, Qt::SortOrder> >::iterator it = m_sortColumns.begin();
    while (it != m_sortColumns.end() ) {
        const QString columnName = (*it).first;
        if ( columnName == column ) {
            m_sortColumns.erase(it);
            break;
        }
    }
}

static QString orderToString( Qt::SortOrder order )
{
    switch ( order ) {
    case Qt::AscendingOrder: return QLatin1String( " ASC" );
    case Qt::DescendingOrder: return QLatin1String( " DESC" );
    };
    qFatal( "Unknown sort order" );
    return QString();
}

void SqlSelectQueryBuilder::addGroupColumn(const QString& column)
{
    m_groupColumns.push_back( column );
}

static QString joinTypeToString( SqlSelectQueryBuilder::JoinType j )
{
    switch ( j ) {
    case SqlSelectQueryBuilder::InnerJoin: return QLatin1String( " INNER JOIN " );
    case SqlSelectQueryBuilder::LeftOuterJoin: return QLatin1String( " LEFT OUTER JOIN " );
    case SqlSelectQueryBuilder::RightOuterJoin: return QLatin1String( " RIGHT OUTER JOIN " );
    case SqlSelectQueryBuilder::FullOuterJoin: return QLatin1String( " FULL OUTER JOIN " );
    case SqlSelectQueryBuilder::CrossJoin: return QLatin1String( " CROSS JOIN " );
    }
    qFatal( "Unknown join type" );
    return QString();
}

SqlQuery& SqlSelectQueryBuilder::query()
{
    if ( !m_assembled ) {
        m_queryString = toString();
        m_assembled = true;
        prepareQuery();
    }
    return m_query;
}

void SqlSelectQueryBuilder::prepareQuery()
{
#ifndef QUERYBUILDER_UNITTEST
    m_query = SqlQueryBuilderBase::prepareQuery( m_queryString );
    for ( int i = 0; i < m_bindValues.size(); ++i ) {
        const QVariant value = m_bindValues.at( i );
        bindValue( i, value );
    }
#endif
}

QString SqlSelectQueryBuilder::toString()
{

    QString queryString;

    QStringList cols;
    typedef QPair<QString, QString> StringPair;
    foreach ( const StringPair &col, m_columns ) {
        if ( col.second.isEmpty() )
            cols.push_back( col.first );
        else {
            QString quotedAs = col.second;
            quotedAs.replace( QLatin1String( "\"" ), QLatin1String( "\\\"" ) );
            cols.push_back( col.first + QLatin1String( " AS \"" ) + quotedAs + QLatin1Char( '"' ) );
        }
    }

    queryString = QLatin1String( "SELECT " );
    if(m_distinct)
        queryString += QLatin1String( "DISTINCT " );
    if (!m_distinctOn.isEmpty()) {
        Q_ASSERT(!m_distinct);
        queryString += QLatin1Literal( "DISTINCT ON(" ) % m_distinctOn % QLatin1Literal( ") " );
    }
    queryString += cols.join( QLatin1String( ", " ) );
    if ( !m_table.isEmpty() ) {
        queryString += QLatin1String( " FROM " );
        queryString += m_table;
    }
    foreach ( const JoinInfo &j, m_joins ) {
        queryString += joinTypeToString( j.type );
        queryString += j.table;
        queryString += QLatin1String( " ON " );
        queryString += conditionToString( j.condition );
    }
    if ( m_whereCondition.hasSubConditions() ) {
        queryString += QLatin1String( " WHERE " );
        queryString += conditionToString( m_whereCondition );
    }

    if ( !m_groupColumns.isEmpty() ) {
        queryString += QLatin1String( " GROUP BY " );
        queryString += m_groupColumns.join( QLatin1String( ", " ) );
    }

    if ( !m_sortColumns.isEmpty() ) {
        queryString += QLatin1String( " ORDER BY " );
        QStringList sortCols;
        typedef QPair<QString, Qt::SortOrder> StringOrderPair;
        foreach ( const StringOrderPair &sortCol, m_sortColumns )
            sortCols.push_back( sortCol.first + orderToString( sortCol.second ) );
        queryString += sortCols.join( QLatin1String( ", " ) );
    }

    if ( !m_lockTablesForUpdate.isEmpty() ) {
        queryString += QLatin1String( " FOR UPDATE OF " );
        queryString += m_lockTablesForUpdate.join( QLatin1String( ", " ) );
        if ( m_lockNoWait )
            queryString += QLatin1String( " NOWAIT" );
    }
    if ( m_limitOffset != -1 ) {
        queryString += QString::fromLatin1( " OFFSET %1 LIMIT %2" ).arg( m_limitOffset ).arg( m_limitLength );
    }

    return queryString;
}

QVector<QVariant> SqlSelectQueryBuilder::bindValuesList()
{
    return m_bindValues;
}

void SqlSelectQueryBuilder::combineQueries(SqlSelectQueryBuilder &query1, SqlSelectQueryBuilder &query2, const UnionType type)
{
    if ( !m_assembled && m_bindValues.isEmpty() && m_queryString == QString()) {
        const QString unionName = (type == UnionAll) ? QLatin1String(" UNION ALL "):QLatin1String( " UNION " );
        const QString query1String = query1.toString();
        query2.m_bindedValuesOffset = query1.m_bindValues.size();
        m_queryString = query1String + unionName +  query2.toString();
        m_assembled = true;
        m_bindValues = query1.bindValuesList() + query2.bindValuesList();
        prepareQuery();
    }
    else {
        SQLDEBUG << "WARNING: you tried to combine queries in a non-empty SqlSelectQueryBuilder, the queries weren't combined. (SqlSelectQueryBuilder stays inchanged).";
    }
}

void SqlSelectQueryBuilder::addLimit(uint offset, uint length)
{
    m_limitOffset = offset;
    m_limitLength = length;
}

