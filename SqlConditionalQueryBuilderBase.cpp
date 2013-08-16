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
#include "SqlConditionalQueryBuilderBase.h"

#include "SqlExceptions.h"

#include <QStringList>

SqlConditionalQueryBuilderBase::SqlConditionalQueryBuilderBase(const QSqlDatabase& db) :
    SqlQueryBuilderBase( db ),
    m_bindedValuesOffset(0)
{
}

SqlCondition& SqlConditionalQueryBuilderBase::whereCondition()
{
    return m_whereCondition;
}

QString SqlConditionalQueryBuilderBase::registerBindValue(const QVariant& value)
{
    m_bindValues.push_back( value );
    return QLatin1Char( ':' ) + QString::number( m_bindedValuesOffset + m_bindValues.size() - 1 );

}

static QString logicOperatorToString( SqlCondition::LogicOperator op )
{
    switch ( op ) {
        case SqlCondition::And: return QLatin1String( " AND " );
        case SqlCondition::Or:  return QLatin1String( " OR " );
    }
    qFatal( "Unknown logic operator" );
    return QString();
}

static QString compareOperatorToString( SqlCondition::CompareOperator op )
{
    switch ( op ) {
        case SqlCondition::Equals: return QLatin1String( " = " );
        case SqlCondition::NotEquals: return QLatin1String( " <> " );
        case SqlCondition::Is: return QLatin1String( " IS " );
        case SqlCondition::IsNot: return QLatin1String( " IS NOT " );
        case SqlCondition::Less: return QLatin1String( " < " );
        case SqlCondition::LessOrEqual: return QLatin1String( " <= " );
        case SqlCondition::Greater: return QLatin1String( " > " );
        case SqlCondition::GreaterOrEqual: return QLatin1String( " >= " );
        case SqlCondition::Like: return QLatin1String( " LIKE " );
    }
    qFatal( "Unknown compare operator." );
    return QString();
}

QString SqlConditionalQueryBuilderBase::conditionToString(const SqlCondition& condition)
{
  if ( condition.hasSubConditions() ) {
    QStringList conds;
    foreach ( const SqlCondition &c, condition.subConditions() )
      conds << conditionToString( c );
    if ( conds.size() == 1 )
        return conds.first();
    return QLatin1Char( '(' ) + conds.join( logicOperatorToString( condition.m_logicOp ) ) + QLatin1Char( ')' );
  } else {
    QString stmt = condition.m_column;
    stmt += compareOperatorToString( condition.m_compareOp );
    if ( condition.m_comparedColumn.isEmpty() ) {
      if ( condition.m_comparedValue.isValid() ) {
          if ( condition.m_comparedValue.userType() == qMetaTypeId<SqlNowType>() )
              stmt += currentDateTime();
          else
              stmt += registerBindValue( condition.m_comparedValue );
      } else if ( !condition.m_placeholder.isEmpty() ) {
          stmt += condition.m_placeholder;
      } else {
        stmt += QLatin1String( "NULL" );
      }
    } else {
      stmt += condition.m_comparedColumn;
    }
    return stmt;
  }
}

