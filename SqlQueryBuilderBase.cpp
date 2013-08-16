/*
    Copyright (C) 2011-2013 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Volker Krause <volker.krause@kdab.com>

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
