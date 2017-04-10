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
    m_columnNames.push_back(columnName);
    m_values.insert(columnName, value);
}

void SqlInsertQueryBuilder::addColumn(const QString &columnName)
{
    m_columnNames.push_back(columnName);
}

void SqlInsertQueryBuilder::setToDefaultValues()
{
    m_values.clear();
}


SqlQuery& SqlInsertQueryBuilder::query()
{
    if ( !m_assembled ) {

        m_queryString = QLatin1String( "INSERT INTO " );
        m_queryString += m_table;

#ifndef QUERYBUILDER_UNITTEST
        bool bindValues = false;
#endif

        if ( m_columnNames.isEmpty() ) { // no columns = all default
            m_queryString += QLatin1String(" DEFAULT VALUES");
        } else {
            if ( !m_columnNames.isEmpty() ) { //columns specified
                m_queryString += QLatin1String( " (" );
                Q_FOREACH( const QString& column, m_columnNames ) {
                    m_queryString += column + QLatin1String( "," );
                }
                m_queryString[ m_queryString.length() -1 ] = QLatin1Char(')');
            }
            m_queryString += QLatin1String( " VALUES (" );
            int index = 0;
            foreach (const QString& column, m_columnNames) {
                if (m_values.contains(column)) {
                    const QVariant value = m_values[column];
                    if (value.userType() == qMetaTypeId<SqlNowType>()) {
                        m_queryString += currentDateTime() % QLatin1Char(',');
                    } else {
                        m_queryString += QString::fromLatin1( ":%1," ).arg(index);
                    }
                } else {
                    m_queryString += QLatin1String( "DEFAULT," );
                }
                ++index;
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
                const QVariant value = m_values.values().at( i );
                bindValue( i, value );
            }
        }
#endif
    }
    return m_query;
}

