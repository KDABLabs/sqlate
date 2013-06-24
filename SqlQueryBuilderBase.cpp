/*
* Copyright (C) 2011-2013 Klaralavdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Author: Volker Krause <volker.krause@kdab.com>
*/
#include "SqlQueryBuilderBase.h"

#include "SqlExceptions.h"
#include "SqlSchema.h"
#include "SqlCondition.h"
#include "SqlQueryCache.h"

SqlQueryBuilderBase::SqlQueryBuilderBase(const QSqlDatabase& db) :
  m_db( db ),
  m_query( db ),
  m_assembled( false )
{
}



SqlQueryBuilderBase::~SqlQueryBuilderBase()
{
}


void SqlQueryBuilderBase::setTable(const QString& tableName)
{
    m_table = tableName;
}

void SqlQueryBuilderBase::invalidateQuery()
{
    m_assembled = false;
}

void SqlQueryBuilderBase::exec()
{
    query().exec();
}

void SqlQueryBuilderBase::bindValue(int placeholderIndex, const QVariant& value)
{
    const QString placeholder = QLatin1Char(':') + QString::number( placeholderIndex );

    if (value.userType() == qMetaTypeId<QUuid>()) {
         // Qt SQL drivers don't handle QUuid
        m_query.bindValue( placeholder, value.value<QUuid>().toString() );
    }

    else if (value.userType() == qMetaTypeId<SqlNowType>()) {
        // don't create any bindings for the SqlNow dummytype, it has been handled when assembling the query string already
    }

    else {
        m_query.bindValue( placeholder, value );
    }
}

QString SqlQueryBuilderBase::currentDateTime() const
{
    return QLatin1String("now()");
}

SqlQuery SqlQueryBuilderBase::prepareQuery(const QString& sqlStatement)
{
    if (SqlQueryCache::contains(m_db.connectionName(), sqlStatement)) {
        return SqlQueryCache::query(m_db.connectionName(), sqlStatement);
    }

    SqlQuery q( m_db );
    q.prepare( sqlStatement );
    SqlQueryCache::insert(m_db.connectionName(), sqlStatement, q);
    return q;
}
