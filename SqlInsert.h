/*
    Copyright (C) 2011-2013 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Volker Krause <volker.krause@kdab.com>
        author Andras Mantia <andras.mantia@kdab.com>
        author Jan Dalheimer <jan.dalheimer@kdab.com>

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
#ifndef SQL_INSERT_H
#define SQL_INSERT_H

#include "SqlInternals_p.h"
#include "SqlSchema_p.h"
#include "SqlCondition.h"
#include "SqlInsertQueryBuilder.h"
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
 * @file SqlInsert.h
 * Insert query expression templates.
 */

namespace Sql {

struct ColumnValue
{
    template <typename ColumnT>
    ColumnValue(const ColumnT&, const typename ColumnT::type& value) :
        columnName(ColumnT::sqlName()), value(QVariant::fromValue( value ))
    {
        Sql::warning<boost::is_same<typename ColumnT::type, QDateTime>, UsageOfClientSideTime>::print();
    }

    QString columnName;
    QVariant value;
};

// TODO find out if it would be possible to have an operator= instead
template <typename ColumnT>
ColumnValue operator>>(const ColumnT& col, const typename ColumnT::type& value)
{
    return ColumnValue(col, value);
}

/**
 * Represents a INSERT statement
 * @internal
 * @tparam TableT The table type inserted into.
 */
template <typename TableT>
struct InsertExpr
{
    /**
     * Empty ctor.
     */
    InsertExpr() {}

    /**
     * "Copy" ctor.
     */
    template <typename OtherTableT>
    explicit
    InsertExpr( const InsertExpr<OtherTableT> &other )
    {
        values = other.values;
    }

    /**
     * Creates the INTO part of a INSERT statement.
     * @tparam T The table type to insert into
     */
    template <typename T>
    InsertExpr<T> into( const T& )
    {
        BOOST_MPL_ASSERT(( boost::is_same<TableT, detail::missing> )); // only one from allowed
        return InsertExpr<T>();
    }

    /**
     * Sets columns + values
     */
    InsertExpr<TableT> columns( const QList<ColumnValue>& cols )
    {
        values = cols;
        return *this;
    }
    InsertExpr<TableT> columns( const ColumnValue& col )
    {
        return columns(QList<ColumnValue>() << col);
    }

    /**
     * @internal
     * for unit testing access only
     */
    SqlInsertQueryBuilder queryBuilder() const
    {
        SqlInsertQueryBuilder qb;
        qb.setTable<TableT>();
        foreach (const ColumnValue& col, values) {
            qb.addColumnValue(col.columnName, col.value);
        }
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
    operator SqlInsertQueryBuilder() const
    {
        return queryBuilder();
    }

    QList<ColumnValue> values;
};

/**
 * Creates a INSERT statement.
 */
InsertExpr<detail::missing> insert()
{
    return InsertExpr<detail::missing>();
}

QList<ColumnValue> operator&(const ColumnValue& c1, const ColumnValue& c2)
{
    return QList<ColumnValue>() << c1 << c2;
}
QList<ColumnValue> operator&(const QList<ColumnValue>& c1, const ColumnValue& c2)
{
    return QList<ColumnValue>() << c1 << c2;
}

}

#endif
