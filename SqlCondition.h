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

#ifndef SQLCONDITION_H
#define SQLCONDITION_H

#include "sqlate_export.h"
#include "SqlInternals_p.h"

#include <QString>
#include <QVariant>
#include <QVector>

#include <boost/mpl/assert.hpp>
#include <boost/mpl/not.hpp>
#include <boost/type_traits/is_same.hpp>

/** SQL NULL type, to allow using NULL in template code, rather than falling back to QVariant(). */
struct SqlNullType {};
static const SqlNullType SqlNull = {}; // "Null" is already in use, also in the Sql namespace, so we have to settle for this

/** SQL now type, to allow using server-side current date/time in template code, rahter than hardcoded SQL strings or client-side time. */
struct SqlNowType {};
static const SqlNowType SqlNow = {};
Q_DECLARE_METATYPE(SqlNowType)

/** Dummy type for compile time warnings about usage of client side time. */
struct UsageOfClientSideTime {};


/** Represents a part of a SQL WHERE expression. */
class SQLATE_EXPORT SqlCondition
{
public:
    /** Compare operators to be used in query conditions. */
    enum CompareOperator {
        Equals,
        NotEquals,
        Is,
        IsNot,
        Less,
        LessOrEqual,
        Greater,
        GreaterOrEqual,
        Like
    };

    /** Logic operation to combine multiple conditions. */
    enum LogicOperator {
        And,
        Or
    };

    /** Create an empty condition, with sub-queries combined using @p op. */
    explicit SqlCondition( LogicOperator op = And );

    /**
      Add a condition which compares a column with a given fixed value.
      @param column The column that should be compared.
      @param op The operator used for comparison
      @param value The value @p column is compared to.
    */
    void addValueCondition( const QString &column, CompareOperator op, const QVariant &value );
    template <typename Column>
    inline void addValueCondition( const Column &column, CompareOperator op, const typename Column::type &value )
    {
        Sql::warning<boost::is_same<typename Column::type, QDateTime>, UsageOfClientSideTime>::print();
        addValueCondition( column.name(), op, QVariant::fromValue<typename Column::type>(value));
    }
    template <typename Column>
    inline void addValueCondition( const Column &column, CompareOperator op, SqlNullType )
    {
        // asserting on Column::notNull is too strict, this can be used in combination with outer joins!
        //BOOST_MPL_ASSERT(( boost::mpl::not_<typename Column::notNull> ));
        // TODO idealy we would also static assert on op == Is[Not]
        addValueCondition( column.name(), op, QVariant() );
    }
    template <typename Column>
    inline void addValueCondition( const Column &column, CompareOperator op, SqlNowType now )
    {
        BOOST_MPL_ASSERT(( boost::is_same<typename Column::type, QDateTime> ));
        // TODO this could also be restricted to less/greater than operations
        addValueCondition( column.name(), op, QVariant::fromValue(now) );
    }

    /**
     * Same as addValueCondition, but for defered value binding.
     * @param column The column that should be compared.
     * @param op The operator used for comparison.
     * @param placeholder A placeholder (with leading ':'), not starting with a number.
     */
    void addPlaceholderCondition( const QString &column, CompareOperator op, const QString &placeholder );
    template <typename Column>
    inline void addPlaceholderCondition( const Column &column, CompareOperator op, const QString &placeholder )
    {
        addPlaceholderCondition( column.name(), op, placeholder );
    }

    /**
      Add a condition which compares a column with another column.
      @param column The column that should be compared.
      @param op The operator used for comparison.
      @param column2 The column @p column is compared to.
    */
    void addColumnCondition( const QString &column, CompareOperator op, const QString &column2 );
    template <typename Column1, typename Column2>
    inline void addColumnCondition( const Column1 &column1, CompareOperator op, const Column2 &column2 )
    {
        BOOST_MPL_ASSERT(( boost::is_same<typename Column1::type, typename Column2::type> ));
        addColumnCondition(column1.name(), op, column2.name());
    }

    /**
      Set the case sensitive flag. This is defaulted to true and must be set before calling @func addCondition() or @func addPlaceholderCondition().
      @param isCasesensitive the operands are converted to the same case before comparison
    */
    void setCaseSensitive( const bool isCaseSensitive );
    /**
      returns the value of the case sensitive flag.
    */
    bool isCaseSensitive() const;

    /**
      Add a nested condition.
    */
    void addCondition( const SqlCondition &condition );

    /**
      Set how sub-conditions should be combined, default is And.
    */
    void setLogicOperator( LogicOperator op );

    /**
     * For nested conditions, this returns the list of sub-conditions.
     */
    QVector<SqlCondition> subConditions() const;

    /**
     * Checks if this condition has sub-conditions.
     */
    bool hasSubConditions() const;

private:
    friend class SqlConditionalQueryBuilderBase;
    QVector<SqlCondition> m_subConditions;
    QString m_column;
    QString m_comparedColumn;
    QString m_placeholder;
    QVariant m_comparedValue;
    CompareOperator m_compareOp;
    LogicOperator m_logicOp;
    bool m_isCaseSensitive;
};

Q_DECLARE_TYPEINFO( SqlCondition, Q_MOVABLE_TYPE );

#endif
