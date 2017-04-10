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

#include <boost/mpl/and.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>

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

namespace Sql {
template <typename SubConditionList, SqlCondition::LogicOperator> struct ConditionExpr;
template <typename Lhs, SqlCondition::CompareOperator, typename Rhs> struct ConditionColumnLeaf;
template <typename Lhs, SqlCondition::CompareOperator, typename Rhs> struct ConditionValueLeaf;
template <typename Lhs, SqlCondition::CompareOperator, typename Rhs> struct ConditionPlaceholderLeaf;

namespace detail {

/**
 * Add a ConditionLeaf to an existing SqlCondition for SQL code generation.
 * @internal
 */
template <typename Lhs, SqlCondition::CompareOperator Comp, typename Rhs>
void append_condition( SqlCondition &cond, const ConditionColumnLeaf<Lhs, Comp, Rhs> & )
{
    cond.addColumnCondition( Lhs(), Comp, Rhs() );
}

template <typename Lhs, SqlCondition::CompareOperator Comp, typename Rhs>
void append_condition( SqlCondition &cond, const ConditionValueLeaf<Lhs, Comp, Rhs> &leaf )
{
    cond.addValueCondition( Lhs::name(), Comp, leaf.value );
}

template <typename Lhs, SqlCondition::CompareOperator Comp, typename Rhs>
void append_condition( SqlCondition &cond, const ConditionPlaceholderLeaf<Lhs, Comp, Rhs> &leaf )
{
    cond.addPlaceholderCondition( Lhs::name(), Comp, leaf.placeholder );
}

/**
 * Metafunction to identify condition expressions.
 * @internal
 */
template <typename T>
struct is_condition_expr : boost::mpl::false_ {};

template <typename SubConditionList, SqlCondition::LogicOperator Op>
struct is_condition_expr<ConditionExpr<SubConditionList, Op> > : boost::mpl::true_ {};

/**
 * Metafunction to wrap a single condition leaf into an condition expression.
 * @tparam T a condition leaf or a condition expression
 * @internal
 */
template <typename T>
struct wrap_condition_leaf
{
    typedef typename boost::mpl::if_<is_condition_expr<T>, T, ConditionExpr<boost::mpl::vector<T>, SqlCondition::And> >::type type;
};

template <typename T>
typename boost::enable_if<is_condition_expr<T>, void>::type
assign_condition( SqlCondition &cond, const T &expr ) { cond = expr.condition; }

template <typename T>
typename boost::disable_if<is_condition_expr<T>, void>::type
assign_condition( SqlCondition &cond, const T &leaf ) { append_condition( cond, leaf ); }

}

/**
 * Represents a single node in a conditional expression
 * @internal
 * @tparam Derived CRTP
 * @tparam Lhs Left hand side statement
 * @tparam Comp Comparator type
 * @tparam Rhs Right hand side statement
 */
template <template <typename, SqlCondition::CompareOperator, typename> class Derived, typename Lhs, SqlCondition::CompareOperator Comp, typename Rhs>
struct ConditionLeaf
{
    ConditionLeaf() {}

    /**
     * Logic operators for combining two leafs.
     */
    template <typename ConditionLeaf2>
    ConditionExpr<boost::mpl::vector<Derived<Lhs, Comp, Rhs>, ConditionLeaf2>, SqlCondition::And> operator&&( const ConditionLeaf2& l2 ) const
    {
        ConditionExpr<boost::mpl::vector<Derived<Lhs, Comp, Rhs>, ConditionLeaf2>, SqlCondition::And> newCond;
        newCond.condition.setLogicOperator( SqlCondition::And );
        detail::append_condition( newCond.condition, *static_cast<const Derived<Lhs, Comp, Rhs>*>( this ) );
        detail::append_condition( newCond.condition, l2 );
        return newCond;
    }

    template <typename ConditionLeaf2>
    ConditionExpr<boost::mpl::vector<Derived<Lhs, Comp, Rhs>, ConditionLeaf2>, SqlCondition::Or> operator||( const ConditionLeaf2& l2 ) const
    {
        ConditionExpr<boost::mpl::vector<Derived<Lhs, Comp, Rhs>, ConditionLeaf2>, SqlCondition::Or> newCond;
        newCond.condition.setLogicOperator( SqlCondition::Or );
        detail::append_condition( newCond.condition, *static_cast<const Derived<Lhs, Comp, Rhs>*>( this ) );
        detail::append_condition( newCond.condition, l2 );
        return newCond;
    }
};

/**
 * Represents a single column condition.
 * @internal
 */
template <typename Lhs, SqlCondition::CompareOperator Comp, typename Rhs>
struct ConditionColumnLeaf : ConditionLeaf<ConditionColumnLeaf, Lhs, Comp, Rhs> {};

/**
 * Represents a single value condition.
 * @internal
 */
template <typename Lhs, SqlCondition::CompareOperator Comp, typename Rhs>
struct ConditionValueLeaf : ConditionLeaf<ConditionValueLeaf, Lhs, Comp, Rhs>
{
    QVariant value;
};

/**
 * Represents a single placeholder condition.
 * @internal
 */
template <typename Lhs, SqlCondition::CompareOperator Comp, typename Rhs = detail::missing>
struct ConditionPlaceholderLeaf : ConditionLeaf<ConditionPlaceholderLeaf, Lhs, Comp, Rhs>
{
    QString placeholder;
};

/**
 * Create a placeholder in conditional expressions for later binding.
 * @param name The placeholder name.
 */
struct placeholder {
    explicit placeholder( const QString &name ) : m_name( name ) {}
    explicit placeholder( const char* name ) : m_name( QString::fromLatin1( name ) ) {}

    QString m_name;
};

/**
 * Represents a complex conditional expression
 * @internal
 * @tparam SubConditionList A MPL sequence of conditions
 * @tparam LogicOp The logical operator used to combine those conditions
 */
template <typename SubConditionList, SqlCondition::LogicOperator LogicOp>
struct ConditionExpr
{
    /**
     * Logic operators to add another leaf.
     */
    template <typename Leaf2>
    typename boost::enable_if_c<(LogicOp == SqlCondition::And), ConditionExpr<typename boost::mpl::push_back<SubConditionList, Leaf2>::type, LogicOp> >::type
    operator&&( const Leaf2 &l2 )
    {
        ConditionExpr<typename boost::mpl::push_back<SubConditionList, Leaf2>::type, LogicOp> newCond;
        newCond.condition = condition;
        detail::append_condition( newCond.condition, l2 );
        return newCond;
    }

    template <typename Leaf2>
    typename boost::enable_if_c<(LogicOp == SqlCondition::Or), ConditionExpr<typename boost::mpl::push_back<SubConditionList, Leaf2>::type, LogicOp> >::type
    operator||( const Leaf2 &l2 )
    {
        ConditionExpr<typename boost::mpl::push_back<SubConditionList, Leaf2>::type, LogicOp> newCond;
        newCond.condition = condition;
        detail::append_condition( newCond.condition, l2 );
        return newCond;
    }

    SqlCondition condition;
};


/*
 * Operators for creating ConditionLeaf instances based on at least one column object.
 */

template <typename ColumnT>
typename boost::enable_if<typename ColumnT::is_column, ConditionValueLeaf<ColumnT, SqlCondition::Equals, typename ColumnT::type> >::type
operator==( const ColumnT &, const typename ColumnT::type& value )
{
    ConditionValueLeaf<ColumnT, SqlCondition::Equals, typename ColumnT::type> newCond;
    newCond.value = QVariant::fromValue( value );
    return newCond;
}
template <typename ColumnT1, typename ColumnT2>
typename boost::enable_if<boost::mpl::and_<typename ColumnT1::is_column, typename ColumnT2::is_column>, ConditionColumnLeaf<ColumnT1, SqlCondition::Equals, ColumnT2> >::type
operator==( const ColumnT1&, const ColumnT2& )
{
    BOOST_MPL_ASSERT(( boost::is_same<typename ColumnT1::type, typename ColumnT2::type> )); // only compare columns of the same type
    return ConditionColumnLeaf<ColumnT1, SqlCondition::Equals, ColumnT2>();
}
template <typename ColumnT1>
typename boost::enable_if<typename ColumnT1::is_column, ConditionPlaceholderLeaf<ColumnT1, SqlCondition::Equals> >::type
operator==( const ColumnT1& , const placeholder &p )
{
    ConditionPlaceholderLeaf<ColumnT1, SqlCondition::Equals> c;
    c.placeholder = p.m_name;
    return c;
}

template <typename ColumnT>
typename boost::enable_if<typename ColumnT::is_column, ConditionValueLeaf<ColumnT, SqlCondition::NotEquals, typename ColumnT::type> >::type
operator!=( const ColumnT &, const typename ColumnT::type& value )
{
    ConditionValueLeaf<ColumnT, SqlCondition::NotEquals, typename ColumnT::type> newCond;
    newCond.value = QVariant::fromValue( value );
    return newCond;
}
template <typename ColumnT1, typename ColumnT2>
typename boost::enable_if<boost::mpl::and_<typename ColumnT1::is_column, typename ColumnT2::is_column>, ConditionColumnLeaf<ColumnT1, SqlCondition::NotEquals, ColumnT2> >::type
operator!=( const ColumnT1&, const ColumnT2& )
{
    BOOST_MPL_ASSERT(( boost::is_same<typename ColumnT1::type, typename ColumnT2::type> )); // only compare columns of the same type
    return ConditionColumnLeaf<ColumnT1, SqlCondition::NotEquals, ColumnT2>();
}
template <typename ColumnT1>
typename boost::enable_if<typename ColumnT1::is_column, ConditionPlaceholderLeaf<ColumnT1, SqlCondition::NotEquals> >::type
operator!=( const ColumnT1& , const placeholder &p )
{
    ConditionPlaceholderLeaf<ColumnT1, SqlCondition::NotEquals> c;
    c.placeholder = p.m_name;
    return c;
}

template <typename ColumnT>
typename boost::enable_if<typename ColumnT::is_column, ConditionValueLeaf<ColumnT, SqlCondition::Less, typename ColumnT::type> >::type
operator<( const ColumnT &, const typename ColumnT::type& value )
{
    ConditionValueLeaf<ColumnT, SqlCondition::Less, typename ColumnT::type> newCond;
    newCond.value = QVariant::fromValue( value );
    return newCond;
}
template <typename ColumnT1, typename ColumnT2>
typename boost::enable_if<boost::mpl::and_<typename ColumnT1::is_column, typename ColumnT2::is_column>, ConditionColumnLeaf<ColumnT1, SqlCondition::Less, ColumnT2> >::type
operator<( const ColumnT1&, const ColumnT2& )
{
    BOOST_MPL_ASSERT(( boost::is_same<typename ColumnT1::type, typename ColumnT2::type> )); // only compare columns of the same type
    return ConditionColumnLeaf<ColumnT1, SqlCondition::Less, ColumnT2>();
}
template <typename ColumnT1>
typename boost::enable_if<typename ColumnT1::is_column, ConditionPlaceholderLeaf<ColumnT1, SqlCondition::Less> >::type
operator<( const ColumnT1& , const placeholder &p )
{
    ConditionPlaceholderLeaf<ColumnT1, SqlCondition::Less> c;
    c.placeholder = p.m_name;
    return c;
}

template <typename ColumnT>
typename boost::enable_if<typename ColumnT::is_column, ConditionValueLeaf<ColumnT, SqlCondition::LessOrEqual, typename ColumnT::type> >::type
operator<=( const ColumnT &, const typename ColumnT::type& value )
{
    ConditionValueLeaf<ColumnT, SqlCondition::LessOrEqual, typename ColumnT::type> newCond;
    newCond.value = QVariant::fromValue( value );
    return newCond;
}
template <typename ColumnT1, typename ColumnT2>
typename boost::enable_if<boost::mpl::and_<typename ColumnT1::is_column, typename ColumnT2::is_column>, ConditionColumnLeaf<ColumnT1, SqlCondition::LessOrEqual, ColumnT2> >::type
operator<=( const ColumnT1&, const ColumnT2& )
{
    BOOST_MPL_ASSERT(( boost::is_same<typename ColumnT1::type, typename ColumnT2::type> )); // only compare columns of the same type
    return ConditionColumnLeaf<ColumnT1, SqlCondition::LessOrEqual, ColumnT2>();
}
template <typename ColumnT1>
typename boost::enable_if<typename ColumnT1::is_column, ConditionPlaceholderLeaf<ColumnT1, SqlCondition::LessOrEqual> >::type
operator<=( const ColumnT1& , const placeholder &p )
{
    ConditionPlaceholderLeaf<ColumnT1, SqlCondition::LessOrEqual> c;
    c.placeholder = p.m_name;
    return c;
}

template <typename ColumnT>
typename boost::enable_if<typename ColumnT::is_column, ConditionValueLeaf<ColumnT, SqlCondition::Greater, typename ColumnT::type> >::type
operator>( const ColumnT &, const typename ColumnT::type& value )
{
    ConditionValueLeaf<ColumnT, SqlCondition::Greater, typename ColumnT::type> newCond;
    newCond.value = QVariant::fromValue( value );
    return newCond;
}
template <typename ColumnT1, typename ColumnT2>
typename boost::enable_if<boost::mpl::and_<typename ColumnT1::is_column, typename ColumnT2::is_column>, ConditionColumnLeaf<ColumnT1, SqlCondition::Greater, ColumnT2> >::type
operator>( const ColumnT1&, const ColumnT2& )
{
    BOOST_MPL_ASSERT(( boost::is_same<typename ColumnT1::type, typename ColumnT2::type> )); // only compare columns of the same type
    return ConditionColumnLeaf<ColumnT1, SqlCondition::Greater, ColumnT2>();
}
template <typename ColumnT1>
typename boost::enable_if<typename ColumnT1::is_column, ConditionPlaceholderLeaf<ColumnT1, SqlCondition::Greater> >::type
operator>( const ColumnT1& , const placeholder &p )
{
    ConditionPlaceholderLeaf<ColumnT1, SqlCondition::Greater> c;
    c.placeholder = p.m_name;
    return c;
}

template <typename ColumnT>
typename boost::enable_if<typename ColumnT::is_column, ConditionValueLeaf<ColumnT, SqlCondition::GreaterOrEqual, typename ColumnT::type> >::type
operator>=( const ColumnT &, const typename ColumnT::type& value )
{
    ConditionValueLeaf<ColumnT, SqlCondition::GreaterOrEqual, typename ColumnT::type> newCond;
    newCond.value = QVariant::fromValue( value );
    return newCond;
}
template <typename ColumnT1, typename ColumnT2>
typename boost::enable_if<boost::mpl::and_<typename ColumnT1::is_column, typename ColumnT2::is_column>, ConditionColumnLeaf<ColumnT1, SqlCondition::GreaterOrEqual, ColumnT2> >::type
operator>=( const ColumnT1&, const ColumnT2& )
{
    BOOST_MPL_ASSERT(( boost::is_same<typename ColumnT1::type, typename ColumnT2::type> )); // only compare columns of the same type
    return ConditionColumnLeaf<ColumnT1, SqlCondition::GreaterOrEqual, ColumnT2>();
}
template <typename ColumnT1>
typename boost::enable_if<typename ColumnT1::is_column, ConditionPlaceholderLeaf<ColumnT1, SqlCondition::GreaterOrEqual> >::type
operator>=( const ColumnT1& , const placeholder &p )
{
    ConditionPlaceholderLeaf<ColumnT1, SqlCondition::GreaterOrEqual> c;
    c.placeholder = p.m_name;
    return c;
}

/**
 * Create column IS NOT NULL condition.
 * @tparam ColumnT The column type.
 */
template <typename ColumnT>
ConditionValueLeaf<ColumnT, SqlCondition::IsNot, QVariant>
isNotNull( const ColumnT & )
{
    return ConditionValueLeaf<ColumnT, SqlCondition::IsNot, QVariant>();
}

/**
 * Create column IS NULL condition.
 * @tparam ColumnT The column type.
 */
template <typename ColumnT>
ConditionValueLeaf<ColumnT, SqlCondition::Is, QVariant>
isNull( const ColumnT & )
{
    BOOST_MPL_ASSERT(( boost::mpl::not_<typename ColumnT::notNull> )); // IS NULL conditions on NOT NULL columns don't make sense
    return ConditionValueLeaf<ColumnT, SqlCondition::Is, QVariant>();
}

/**
 * Creates a LIKE condition.
 * @tparam ColumnT The column type to match the pattern against, must have a string type.
 */
template <typename ColumnT>
ConditionValueLeaf<ColumnT, SqlCondition::Like, QString>
like( const ColumnT &, const QString &pattern )
{
    BOOST_MPL_ASSERT(( boost::is_same<typename ColumnT::type, QString> )); // pattern matching only works for strings
    ConditionValueLeaf<ColumnT, SqlCondition::Like, QString> c;
    c.value = pattern;
    return c;
}

}

Q_DECLARE_TYPEINFO( SqlCondition, Q_MOVABLE_TYPE );

#endif
