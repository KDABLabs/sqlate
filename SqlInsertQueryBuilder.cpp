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
#include "SqlInsertQueryBuilder.h"

#include "SqlExceptions.h"
#include "Sql.h"

SqlInsertQueryBuilder::SqlInsertQueryBuilder(const QSqlDatabase& db) :
  SqlQueryBuilderBase( db )
{
}

void SqlInsertQueryBuilder::addColumnValue(const QString& columnName, const QVariant& value)
{
    m_columns.push_back( qMakePair( columnName, value ) );
}

void SqlInsertQueryBuilder::addAllValues(const QVector< QVariant >& values)
{
    Q_FOREACH( const QVariant &value, values ) {
        addColumnValue( QLatin1String( "" ), value );
    }
}

void SqlInsertQueryBuilder::addDefaultValues()
{
    m_columns.clear();
}


SqlQuery& SqlInsertQueryBuilder::query()
{
    if ( !m_assembled ) {
        typedef QPair<QString, QVariant> ColumnValuePair;
        m_columnNames.clear();
        m_values.clear();
        foreach ( const ColumnValuePair &col, m_columns ) {
            m_columnNames.push_back( col.first );
            m_values.push_back( col.second );
        }

        m_queryString = QLatin1String( "INSERT INTO " );
        m_queryString += m_table;
#ifndef QUERYBUILDER_UNITTEST
        bool bindValues = false;
#endif
        if ( m_columns.isEmpty() ) { //default values
            m_queryString += QLatin1String(" DEFAULT VALUES");
        } else {
            if ( !m_columnNames.join( QLatin1String("") ).trimmed().isEmpty() ) { //columns specified
              m_queryString += QLatin1String( " (" );
              Q_FOREACH( const QString& column, m_columnNames ) {
                  m_queryString += column + QLatin1String( "," );
              }
              m_queryString[ m_queryString.length() -1 ] = QLatin1Char(')');
            }

            m_queryString += QLatin1String( " VALUES (" );
            for (int i = 0; i < m_values.size(); ++i ) {
                if (m_values.at(i).userType() == qMetaTypeId<SqlNowType>())
                    m_queryString += currentDateTime() % QLatin1Char(',');
                else
                    m_queryString += QString::fromLatin1(  ":%1," ).arg(i);
            }
            m_queryString[ m_queryString.length() -1 ] = QLatin1Char(')');
#ifndef QUERYBUILDER_UNITTEST
            bindValues = true;
#endif
        }

        m_assembled = true;

#ifndef QUERYBUILDER_UNITTEST
        m_query = prepareQuery( m_queryString );

        if ( bindValues ) {
            for ( int i = 0; i < m_values.size(); ++i ) {
                const QVariant value = m_values.at( i );
                bindValue( i, value );
            }
        }
#endif
    }
    return m_query;
}

