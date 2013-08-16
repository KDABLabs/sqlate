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
#include "SqlUpdateQueryBuilder.h"
#include "SqlExceptions.h"

#include "Sql.h"

SqlUpdateQueryBuilder::SqlUpdateQueryBuilder(const QSqlDatabase& db) :
  SqlConditionalQueryBuilderBase( db ),
  m_includeSubTables( true )
{
}

void SqlUpdateQueryBuilder::addColumnValue(const QString& columnName, const QVariant& value)
{
    m_columns.push_back( qMakePair( columnName, value ) );
}

void SqlUpdateQueryBuilder::setIncludesubTales(bool includeSubTables)
{
    m_includeSubTables = includeSubTables;
}

SqlQuery& SqlUpdateQueryBuilder::query()
{
    if ( !m_assembled ) {
        QStringList cols;
        m_queryString = QLatin1String( "UPDATE " );
        if (!m_includeSubTables) {
            m_queryString += QLatin1String( "ONLY " );
        }
        m_queryString += m_table;

        m_queryString += QLatin1String( " SET " );
        m_bindValues.clear();
        typedef QPair<QString, QVariant> ColumnValuePair;
        foreach ( const ColumnValuePair &col, m_columns ) {
            m_queryString += col.first;
            m_queryString += QLatin1String( " = " );
            if ( col.second.userType() == qMetaTypeId<SqlNowType>() )
                m_queryString += currentDateTime();
            else
                m_queryString += registerBindValue( col.second );
            m_queryString += QLatin1String(", ");
        }
        if ( m_queryString.endsWith( QLatin1String(", ") ) )
            m_queryString.remove( m_queryString.length() - 2, 2);

        if ( m_whereCondition.hasSubConditions() ) {
            m_queryString += QLatin1String( " WHERE " );
            m_queryString += conditionToString( m_whereCondition );
        }

        m_queryString = m_queryString.trimmed();
        m_assembled = true;

#ifndef QUERYBUILDER_UNITTEST
        m_query = prepareQuery( m_queryString );

        for ( int i = 0; i < m_bindValues.size(); ++i ) {
            const QVariant value = m_bindValues.at( i );
            bindValue( i, value );
        }
#endif
    }
    return m_query;
}

QStringList SqlUpdateQueryBuilder::columnNames() const
{
    QStringList names;
    typedef QPair<QString, QVariant> ColumnValuePair;
    foreach ( const ColumnValuePair &col, m_columns ) {
        names << col.first;
    }
    return names;
}
