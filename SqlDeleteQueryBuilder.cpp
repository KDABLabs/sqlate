/*
* Copyright (C) 2011-2013 Klaralavdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Author: Andras Mantia <andras.mantia@kdab.com>
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

