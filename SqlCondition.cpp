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
#include "SqlCondition.h"

SqlCondition::SqlCondition(SqlCondition::LogicOperator op) :
  m_compareOp( Equals ),
  m_logicOp( op ),
  m_isCaseSensitive( true )
{
}

void SqlCondition::addValueCondition(const QString& column, SqlCondition::CompareOperator op, const QVariant& value)
{
    Q_ASSERT( !column.isEmpty() );
    SqlCondition c;
    c.m_compareOp = op;
    if ( (!m_isCaseSensitive) && value.type() == QVariant::String )
    {
        c.m_comparedValue = value.toString().toLower();
        c.m_column = QString::fromLatin1( "LOWER( %1 ) " ).arg( column );
    }
    else
    {
        c.m_comparedValue = value;
        c.m_column = column;
    }
    m_subConditions.push_back( c );
}

void SqlCondition::addPlaceholderCondition(const QString& column, SqlCondition::CompareOperator op, const QString& placeholder)
{
    Q_ASSERT( !column.isEmpty() );
    Q_ASSERT( placeholder.size() >= 2 );
    Q_ASSERT( placeholder.startsWith( QLatin1Char( ':' ) ) );
    Q_ASSERT( !placeholder.at( 1 ).isDigit() );
    SqlCondition c;
    c.m_compareOp = op;
    if ( m_isCaseSensitive )
    {
        c.m_placeholder = placeholder;
        c.m_column = column;
    }
    else
    {
        c.m_placeholder = placeholder.toLower();
        c.m_column = QString::fromLatin1( "LOWER( %1 ) " ).arg( column );
    }
    m_subConditions.push_back( c );
}

void SqlCondition::addColumnCondition(const QString& column, SqlCondition::CompareOperator op, const QString& column2)
{
  Q_ASSERT( !column.isEmpty() );
  Q_ASSERT( !column2.isEmpty() );
  SqlCondition c;
  c.m_column = column;
  c.m_comparedColumn = column2;
  c.m_compareOp = op;
  m_subConditions.push_back( c );
}

void SqlCondition::addCondition(const SqlCondition& condition)
{
    m_subConditions.push_back( condition );
}

void SqlCondition::setLogicOperator(SqlCondition::LogicOperator op)
{
    m_logicOp = op;
}

QVector< SqlCondition > SqlCondition::subConditions() const
{
    return m_subConditions;
}

bool SqlCondition::hasSubConditions() const
{
    return !m_subConditions.isEmpty();
}

void SqlCondition::setCaseSensitive( const bool isCaseSensitive )
{
    m_isCaseSensitive = isCaseSensitive;
}

bool SqlCondition::isCaseSensitive() const
{
    return m_isCaseSensitive;
}
