/*
    Copyright (C) 2011-2017 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.com,
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
#ifndef SQL_DELETE_H
#define SQL_DELETE_H

#include "SqlCondition.h"
#include "SqlDeleteQueryBuilder.h"
#include "SqlInternals_p.h"

#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_same.hpp>

/**
 * @file SqlDelete.h
 * Delete query expression templates.
 */

namespace Sql {

/**
 * Represents a DELETE statement
 * @internal
 * @tparam TableT The table type inserted into.
 */
template <typename TableT, typename WhereExprT>
struct DeleteExpr
{
    /**
     * Empty ctor.
     */
    DeleteExpr() {}

    /**
     * "Copy" ctor.
     */
    template <typename OtherTableT, typename OtherWhereExprT>
    explicit
    DeleteExpr( const DeleteExpr<OtherTableT, OtherWhereExprT> &other )
    {
        whereCondition = other.whereCondition;
    }

    /**
     * Creates the INTO part of a INSERT statement.
     * @tparam T The table type to insert into
     */
    template <typename T>
    DeleteExpr<T, WhereExprT> from( const T& )
    {
        BOOST_MPL_ASSERT(( boost::is_same<TableT, detail::missing> )); // only one from allowed
        return DeleteExpr<T, WhereExprT>();
    }

    /**
     * Create the WHERE part of a SELECT statement.
     * @tparam W The where experession
     */
    template <typename W>
    DeleteExpr<TableT, typename detail::wrap_condition_leaf<W>::type> where( const W& cond )
    {
        BOOST_MPL_ASSERT(( boost::is_same<WhereExprT, detail::missing> )); // only one where allowed
        DeleteExpr<TableT, typename detail::wrap_condition_leaf<W>::type> s( *this );
        detail::assign_condition( s.whereCondition, cond );
        return s;
    }

    /**
     * @internal
     * for unit testing access only
     */
    SqlDeleteQueryBuilder queryBuilder() const
    {
        SqlDeleteQueryBuilder qb;
        qb.setTable<TableT>();
        qb.whereCondition() = whereCondition;
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
    operator SqlDeleteQueryBuilder() const
    {
        return queryBuilder();
    }

    SqlCondition whereCondition;
};

/**
 * Creates a DELETE statement. We can't use delete() as it is a reserved keyword.
 */
DeleteExpr<detail::missing, detail::missing> del()
{
    return DeleteExpr<detail::missing, detail::missing>();
}

}

#endif
