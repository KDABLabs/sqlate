/*
* Copyright (C) 2011-2013 Klaralavdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Author: Volker Krause <volker.krause@kdab.com>
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
