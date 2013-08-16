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
#include "SqlDeleteQueryBuilder.h"
#include "SqlExceptions.h"

#include "Sql.h"

#include <QUuid>

SqlDeleteQueryBuilder::SqlDeleteQueryBuilder(const QSqlDatabase& db) :
  SqlConditionalQueryBuilderBase( db ),
  m_includeSubTables( true )
{
}

void SqlDeleteQueryBuilder::setIncludeSubTables(bool includeSubTables)
{
    m_includeSubTables = includeSubTables;
}

SqlQuery& SqlDeleteQueryBuilder::query()
{
    if ( !m_assembled ) {
        QStringList cols;
        m_queryString = QLatin1String( "DELETE FROM " );
        if ( !m_includeSubTables ) {
            m_queryString += QLatin1String( "ONLY " );
        }
        m_queryString += m_table;

        m_bindValues.clear();
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
            // FIXME: similar code in the other query builders, we probably want that in the base class
            if ( qstrcmp( value.typeName(), QMetaType::typeName( qMetaTypeId<QUuid>() ) ) == 0 )
                m_query.bindValue( QLatin1Char( ':' ) + QString::number( i ), value.value<QUuid>().toString() ); // Qt SQL drivers don't handle QUuid
            else
                m_query.bindValue( QLatin1Char( ':' ) + QString::number( i ), value );
        }
#endif
    }
    return m_query;
}

