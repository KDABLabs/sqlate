/*
    Copyright (C) 2011-2013 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Volker Krause <volker.krause@kdab.com>
        author Andras Mantia <andras.mantia@kdab.com>

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
#ifndef SQL_SELECT_H
#define SQL_SELECT_H

#include "SqlInternals_p.h"
#include "SqlSchema_p.h"
#include "SqlCondition.h"
#include "SqlSelectQueryBuilder.h"
#include "SqlGlobal.h"

#include <boost/mpl/and.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/size.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/static_assert.hpp>

/**
 * @file SqlSelect.h
 * Select query expression templates.
 */

namespace Sql {

template <typename SubConditionList, SqlCondition::LogicOperator> struct ConditionExpr;
template <typename Lhs, SqlCondition::CompareOperator, typename Rhs> struct ConditionColumnLeaf;
template <typename Lhs, SqlCondition::CompareOperator, typename Rhs> struct ConditionValueLeaf;
template <typename Lhs, SqlCondition::CompareOperator, typename Rhs> struct ConditionPlaceholderLeaf;

namespace detail {

/**
 * MPL for_each accumulator to add columns to the query builder.
 * @internal
 */
struct columns_to_querybuilder
{
    explicit columns_to_querybuilder( SqlSelectQueryBuilder &qb ) : m_qb( qb ) {}
    template<typename C> void operator()( wrap<C> )
    {
        m_qb.addColumn( C::name() );
    }

    SqlSelectQueryBuilder &m_qb;
};

/**
 * MPL for_each accumulator to add group by columnt to the query builder.
 * @internal
 */
struct groupby_to_querybuilder
{
    explicit groupby_to_querybuilder( SqlSelectQueryBuilder &qb ) : m_qb( qb ) {}
    template <typename C> void operator() ( wrap<C> )
    {
        m_qb.addGroupColumn( C::name() );
    }
    SqlSelectQueryBuilder &m_qb;
};

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

/**
 * Container for runtime information of joins.
 * @internal
 */
struct JoinInfo {
    SqlSelectQueryBuilder::JoinType type;
    QString table;
    SqlCondition condition;
};

/**
 * Container for runtime information about ordering
 * @internal
 */
struct OrderInfo {
    QString column;
    Qt::SortOrder order;
};

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


/**
 * Represents a JOIN statement
 * @internal
 * @tparam TableT The table to join with
 * @tparam JoinCond The condition to join on
 */
template <typename TableT, typename JoinCond>
struct JoinExpr
{
};


/**
 * Represents a SELECT statement
 * @internal
 * @tparam ColumnList A MPL sequence of selected column types.
 * @tparam TableT The table type selected from.
 * @tparam JoinList A list of join expressions
 * @tparam GroupByList A MPL sequence of columns to group.
 * @tparam WhereExprT The WHERE expression.
 */
template <typename ColumnList, typename TableT, typename JoinList, typename WhereExprT,
          typename GroupByList = boost::mpl::vector<>,
          typename SortList = boost::mpl::vector<> >
struct SelectExpr
{
    /**
     * Empty ctor.
     */
    SelectExpr() {}

    /**
     * "Copy" ctor.
     */
    template <typename OtherColumnList, typename OtherTableT, typename OtherJoinList, typename OtherWhereExprT, typename OtherGroupByList, typename OtherSortList>
    explicit
    SelectExpr( const SelectExpr<OtherColumnList, OtherTableT, OtherJoinList, OtherWhereExprT, OtherGroupByList, OtherSortList> &other )
    {
        whereCondition = other.whereCondition;
        joinInfos = other.joinInfos;
        orderInfos = other.orderInfos;
    }

    /**
     * Creates the FROM part of a SELECT statement.
     * @tparam T The table type to select from
     */
    template <typename T>
    SelectExpr<ColumnList, T, JoinList, WhereExprT, GroupByList, SortList> from( const T& )
    {
        BOOST_MPL_ASSERT(( boost::is_same<TableT, detail::missing> )); // only one from allowed
        return SelectExpr<ColumnList, T, JoinList, WhereExprT, GroupByList, SortList>();
    }

    /**
     * Create the WHERE part of a SELECT statement.
     * @tparam W The where experession
     */
    template <typename W>
    SelectExpr<ColumnList, TableT, JoinList, typename detail::wrap_condition_leaf<W>::type, GroupByList, SortList> where( const W& cond )
    {
        BOOST_MPL_ASSERT(( boost::is_same<WhereExprT, detail::missing> )); // only one where allowed
        SelectExpr<ColumnList, TableT, JoinList, typename detail::wrap_condition_leaf<W>::type, GroupByList, SortList> s( *this );
        detail::assign_condition( s.whereCondition, cond );
        return s;
    }

    /**
     * Add a JOIN expression
     * @tparam T The table to join.
     * @tparam JoinCond The condition to join on.
     */
    #define MAKE_JOIN_METHOD( methodName, joinType ) \
    template <typename T, typename JoinCond> \
    SelectExpr<ColumnList, TableT, typename boost::mpl::push_back<JoinList, JoinExpr<T, JoinCond> >::type , WhereExprT, GroupByList, SortList> \
    methodName ( const T&, const JoinCond &cond ) \
    { \
        BOOST_MPL_ASSERT(( boost::is_same<WhereExprT, detail::missing> )); /* join comes before WHERE */ \
        SelectExpr<ColumnList, TableT, typename boost::mpl::push_back<JoinList, JoinExpr<T, JoinCond> >::type, WhereExprT, GroupByList, SortList> s( *this ); \
        detail::JoinInfo ji; \
        ji.type = SqlSelectQueryBuilder:: joinType; \
        ji.table = T::tableName(); \
        detail::assign_condition( ji.condition, cond ); \
        s.joinInfos.push_back( ji ); \
        return s; \
    }
    MAKE_JOIN_METHOD( innerJoin, InnerJoin )
    MAKE_JOIN_METHOD( leftOuterJoin, LeftOuterJoin )
    MAKE_JOIN_METHOD( rightOuterJoin, RightOuterJoin )
    MAKE_JOIN_METHOD( fullOuterJoin, FullOuterJoin )
    MAKE_JOIN_METHOD( crossJoin, CrossJoin )
    #undef MAKE_JOIN_METHOD

    /**
     * Add GROUP BY expressions.
     * @tparam Tn Column to group.
     */
    #ifndef SQL_GROUP_BY_MAX_SIZE
    #define SQL_GROUP_BY_MAX_SIZE 5
    #endif
    #define CONST_REF(z, n, unused) const T ## n &
    #define GROUP_BY_IMPL(z, n, unused) \
    template <BOOST_PP_ENUM_PARAMS(n, typename T)> \
    SelectExpr<ColumnList, TableT, JoinList, WhereExprT, boost::mpl::vector<BOOST_PP_ENUM_PARAMS(n, T)>, SortList> \
    groupBy( BOOST_PP_ENUM(n, CONST_REF, ~) ) \
    { \
        BOOST_MPL_ASSERT(( boost::mpl::not_<boost::is_same<TableT, detail::missing> > )); /* FROM comes before GROUP BY */ \
        BOOST_STATIC_ASSERT(( boost::mpl::size<GroupByList>::value == 0 )); /* only one GROUP BY */ \
        return SelectExpr<ColumnList, TableT, JoinList, WhereExprT, boost::mpl::vector<BOOST_PP_ENUM_PARAMS(n, T)>, SortList>( *this ); \
    }
    BOOST_PP_REPEAT_FROM_TO(1, SQL_GROUP_BY_MAX_SIZE, GROUP_BY_IMPL, ~)

    #undef GROUP_BY_IMPL
    #undef CONST_REF


    /**
     * Add ORDER BY expressions.
     */
    #ifndef SQL_ORDER_BY_MAX_SIZE
    #define SQL_ORDER_BY_MAX_SIZE 3
    #endif

    #define RECORD_ORDER_INFO(z, n, unused) \
    { \
        detail::OrderInfo oi; \
        oi.column = T ## n :: name(); \
        oi.order = Qt::AscendingOrder; \
        s.orderInfos.push_back( oi ); \
    }
    #define CONST_REF(z, n, unused) const T ## n &
    #define ORDER_BY_IMPL(z, n, unused) \
    template <BOOST_PP_ENUM_PARAMS(n, typename T)> \
    SelectExpr<ColumnList, TableT, JoinList, WhereExprT, GroupByList, boost::mpl::vector<BOOST_PP_ENUM_PARAMS(n, T)> > orderBy( \
        BOOST_PP_ENUM(n, CONST_REF, ~) ) \
    { \
        BOOST_MPL_ASSERT(( boost::mpl::not_<boost::is_same<TableT, detail::missing> > )); /* FROM comes before ORDER BY */ \
        BOOST_STATIC_ASSERT(( boost::mpl::size<SortList>::value == 0 )); /* only one ORDER BY */ \
        SelectExpr<ColumnList, TableT, JoinList, WhereExprT, GroupByList, boost::mpl::vector<BOOST_PP_ENUM_PARAMS(n, T)> > s( *this ); \
        BOOST_PP_REPEAT(n, RECORD_ORDER_INFO, ~) \
        return s; \
    }
    BOOST_PP_REPEAT_FROM_TO(1, SQL_ORDER_BY_MAX_SIZE, ORDER_BY_IMPL, ~)

    #undef ORDER_BY_IMPL
    #undef CONST_REF
    #undef RECORD_ORDER_INFO

    #define RECORD_ORDER_INFO(z, n, unused) \
    { \
        detail::OrderInfo oi; \
        oi.column = T ## n :: name(); \
        oi.order = order ## n; \
        s.orderInfos.push_back( oi ); \
    }
    #define CONST_REF_AND_ORDER(z, n, unused) const T ## n &, Qt::SortOrder order ## n
    #define ORDER_BY_IMPL(z, n, unused) \
    template <BOOST_PP_ENUM_PARAMS(n, typename T)> \
    SelectExpr<ColumnList, TableT, JoinList, WhereExprT, GroupByList, boost::mpl::vector<BOOST_PP_ENUM_PARAMS(n, T)> > orderBy( \
        BOOST_PP_ENUM(n, CONST_REF_AND_ORDER, ~) ) \
    { \
        BOOST_MPL_ASSERT(( boost::mpl::not_<boost::is_same<TableT, detail::missing> > )); /* FROM comes before ORDER BY */ \
        BOOST_STATIC_ASSERT(( boost::mpl::size<SortList>::value == 0 )); /* only one ORDER BY */ \
        SelectExpr<ColumnList, TableT, JoinList, WhereExprT, GroupByList, boost::mpl::vector<BOOST_PP_ENUM_PARAMS(n, T)> > s( *this ); \
        BOOST_PP_REPEAT(n, RECORD_ORDER_INFO, ~) \
        return s; \
    }
    BOOST_PP_REPEAT_FROM_TO(1, SQL_ORDER_BY_MAX_SIZE, ORDER_BY_IMPL, ~)

    #undef ORDER_BY_IMPL
    #undef CONST_REF_AND_ORDER
    #undef RECORD_ORDER_INFO

    /**
     * @internal
     * for unit testing access only
     */
    SqlSelectQueryBuilder queryBuilder() const
    {
        SqlSelectQueryBuilder qb;
        boost::mpl::for_each<ColumnList, detail::wrap<boost::mpl::placeholders::_1> >( detail::columns_to_querybuilder( qb ) );
        qb.setTable<TableT>();
        foreach ( const detail::JoinInfo& ji, joinInfos )
            qb.addJoin( ji.type, ji.table, ji.condition );
        qb.whereCondition() = whereCondition;
        boost::mpl::for_each<GroupByList, detail::wrap<boost::mpl::placeholders::_1> >( detail::groupby_to_querybuilder( qb ) );
        foreach ( const detail::OrderInfo &oi, orderInfos )
            qb.addSortColumn( oi.column, oi.order );
        return qb;
    }

    /**
     * Returns a prepared QSqlQuery ready for execution.
     */
    operator SqlQuery() const
    {
        return queryBuilder().query();
    }

    /**
     * Returns the pre-filled dynamic query builder.
     * This is useful if intermediate queries have to be stored for extension etc.
     * Since we don't have support for the auto keyword everywhere yet, that's currently our best option.
     */
    operator SqlSelectQueryBuilder() const
    {
        return queryBuilder();
    }

    SqlCondition whereCondition;
    QVector<detail::JoinInfo> joinInfos;
    QVector<detail::OrderInfo> orderInfos;
};


/**
 * Creates a SELECT statement.
 * @tparam ColumnList A MPL sequence of the columns to select
 */

#ifndef SQL_SELECT_MAX_SIZE
#define SQL_SELECT_MAX_SIZE 45
#endif

#define CONST_REF(z, n, unused) const T ## n &
#define SELECT_IMPL(z, n, unused) \
    template <BOOST_PP_ENUM_PARAMS(n, typename T)> \
    SelectExpr<boost::mpl::vector<BOOST_PP_ENUM_PARAMS(n, T)>,\
        detail::missing, boost::mpl::vector<>, detail::missing> select( \
    BOOST_PP_ENUM(n, CONST_REF, ~) ) { \
        return SelectExpr<boost::mpl::vector<BOOST_PP_ENUM_PARAMS(n, T)>, detail::missing, boost::mpl::vector<>, detail::missing>(); \
    }
BOOST_PP_REPEAT_FROM_TO(1, SQL_SELECT_MAX_SIZE, SELECT_IMPL, ~)

#undef SELECT_IMPL
#undef CONST_REF

//Operators to fix compilation under MSVC10, that picks up the wrong overloads

template <typename T> typename boost::enable_if<boost::is_enum<T>, bool>::type
operator==(int lhs, T rhs)
{
	return lhs == (int)rhs;
}

template <typename T> typename boost::enable_if<boost::is_enum<T>, bool>::type
operator==(T lhs, int rhs )
{
	return rhs == (int)lhs;
}

inline bool operator==(QStringList lhs, QStringList rhs )
{
  return lhs.operator==(rhs);
}

inline bool operator==(const QUuid &lhs, const Sql::StaticUuid &rhs)
{
    return lhs.operator==(rhs);
}

}

#endif
