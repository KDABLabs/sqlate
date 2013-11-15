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

#include "SqlInsertQueryBuilder.h"

#include <boost/mpl/and.hpp>
#include <boost/utility/enable_if.hpp>

/**
 * @file SqlInsert.h
 * Insert query expression templates.
 *
 * @todo Subquery (INSERT INTO a SELECT * FROM b)
 */

namespace Sql {

struct ColumnValue
{
    template <typename ColumnT>
    ColumnValue(const ColumnT&, const typename ColumnT::type& value) :
        columnName(ColumnT::sqlName()), value(QVariant::fromValue( value )), isDefault(false)
    {
        Sql::warning<boost::is_same<typename ColumnT::type, QDateTime>, UsageOfClientSideTime>::print();
    }

    template <typename ColumnT>
    ColumnValue(const ColumnT&) :
        columnName(ColumnT::sqlName()), isDefault(true)
    {
    }

    ColumnValue(const QString& name) :
        columnName(name), isDefault(true)
    {
    }

    QString columnName;
    QVariant value;
    bool isDefault;
};

// TODO find out if it would be possible to have an operator= instead
template <typename ColumnT>
ColumnValue operator<<(const ColumnT& col, const typename ColumnT::type& value)
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
    InsertExpr() : useDefaultValues(false) {}

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
        values += cols;
        return *this;
    }
    InsertExpr<TableT> columns( const ColumnValue& col )
    {
        return columns(QList<ColumnValue>() << col);
    }

    InsertExpr<TableT> defaultValues()
    {
        useDefaultValues = true;
        return *this;
    }

    /**
     * @internal
     * for unit testing access only
     */
    SqlInsertQueryBuilder queryBuilder() const
    {
        SqlInsertQueryBuilder qb;
        qb.setTable<TableT>();
        if (values.isEmpty() || useDefaultValues) {
            foreach (const ColumnValue& column, values) {
                qb.addColumn(column.columnName);
            }
            qb.setToDefaultValues();
        } else {
            foreach (const ColumnValue& col, values) {
                if (col.isDefault) {
                    qb.addColumn(col.columnName);
                } else {
                    qb.addColumnValue(col.columnName, col.value);
                }
            }
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
    bool useDefaultValues;
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

template <typename ColumnT1, typename ColumnT2>
typename boost::enable_if<boost::mpl::and_<typename ColumnT1::is_column, typename ColumnT2::is_column>, QList<ColumnValue> >::type
operator&(const ColumnT1& c1, const ColumnT2& c2)
{
    return operator&(ColumnValue(c1), ColumnValue(c2));
}
template <typename ColumnT>
typename boost::enable_if<typename ColumnT::is_column, QList<ColumnValue> >::type
operator&(const ColumnT& c1, const ColumnValue& c2)
{
    return operator&(ColumnValue(c1), c2);
}
template <typename ColumnT>
typename boost::enable_if<typename ColumnT::is_column, QList<ColumnValue> >::type
operator&(const ColumnValue& c1, const ColumnT& c2)
{
    return operator&(c1, ColumnValue(c2));
}
template <typename ColumnT>
typename boost::enable_if<typename ColumnT::is_column, QList<ColumnValue> >::type
operator&(const QList<ColumnValue>& c1, const ColumnT& c2)
{
    return operator&(c1, ColumnValue(c2));
}

}

#endif
