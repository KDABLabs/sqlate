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

#ifndef SQLCONDITIONALQUERYBUILDERBASE_H
#define SQLCONDITIONALQUERYBUILDERBASE_H

#include "SqlQueryBuilderBase.h"

#include "SqlCondition.h"
#include "sqlate_export.h"

/** Base class for query builders operating having a WHERE condition.
 */
class SQLATE_EXPORT SqlConditionalQueryBuilderBase : public SqlQueryBuilderBase
{
public:
    /// Create a new query builder for the given database
    explicit SqlConditionalQueryBuilderBase( const QSqlDatabase &db = QSqlDatabase::database() );

    /// access to the top-level WHERE condition
    SqlCondition& whereCondition();

protected:
    /** Register a bound value, to be replaced after query preparation
     *  @param value The value to bind
     *  @return A unique placeholder to use in the query.
     */
    QString registerBindValue( const QVariant &value );

    QString conditionToString( const SqlCondition &condition );

protected:
    SqlCondition m_whereCondition;
    QVector<QVariant> m_bindValues;
    int m_bindedValuesOffset; //holds the parameters offset

};

#endif
