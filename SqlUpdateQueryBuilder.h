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
#ifndef SQLUPDATEQUERYBUILDER_H
#define SQLUPDATEQUERYBUILDER_H

#include "sqlate_export.h"
#include "SqlConditionalQueryBuilderBase.h"
#include "SqlInternals_p.h"

#include <boost/mpl/assert.hpp>
#include <boost/mpl/not.hpp>

/** API to create UPDATE queries in a safe way from code, without the risk of
 *  introducing SQL injection vulnerabilities or typos.
 */
class SQLATE_EXPORT SqlUpdateQueryBuilder : public SqlConditionalQueryBuilderBase
{
public:
    /// Create a new query builder for the given database
    explicit SqlUpdateQueryBuilder( const QSqlDatabase &db = QSqlDatabase::database() );

    /// UPDATE ... SET @p columnName = @p value, ...
    void addColumnValue( const QString &columnName, const QVariant &value );

    template <typename Column>
    void addColumnValue( const Column &, const typename Column::type &value )
    {
        Sql::warning<boost::is_same<typename Column::type, QDateTime>, UsageOfClientSideTime>::print();
        addColumnValue( Column::sqlName(), QVariant::fromValue( value ) );
    }
    template <typename Column>
    void addColumnValue( const Column &, SqlNullType )
    {
        BOOST_MPL_ASSERT(( boost::mpl::not_<typename Column::notNull> ));
        addColumnValue( Column::sqlName(), QVariant() );
    }
    template <typename Column>
    void addColumnValue( const Column &, SqlNowType now )
    {
        BOOST_MPL_ASSERT(( boost::is_same<typename Column::type, QDateTime> ));
        addColumnValue( Column::sqlName(), QVariant::fromValue(now) );
    }

    /**
     * If @p includeSubTables is false, the query will be UPDATE <b>ONLY</b> &lt;tableName&gt;
     * The default is @c true.
     */
    void setIncludesubTales( bool includeSubTables );

    /// Returns the created query object, when called first, the query object is assembled and prepared
    /// The method throws an SqlException if there is an error preparing the query.
    SqlQuery& query();

private:
    QStringList columnNames() const; //used for testing

private:
    friend class UpdateQueryBuilderTest;
    QVector<QPair<QString, QVariant> > m_columns;
    bool m_includeSubTables;
};

#endif
