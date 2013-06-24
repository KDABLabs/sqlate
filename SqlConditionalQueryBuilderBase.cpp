/*
* Copyright (C) 2011-2013 Klaralavdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Author: Volker Krause <volker.krause@kdab.com>
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

