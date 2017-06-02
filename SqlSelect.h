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
