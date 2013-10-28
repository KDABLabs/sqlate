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
#ifndef SQLINSERTQUERYBUILDER_H
#define SQLINSERTQUERYBUILDER_H

#include "sqlate_export.h"
#include "SqlQueryBuilderBase.h"
#include "SqlInternals_p.h"

#include <QDateTime>
#include <QStringList>

#include <boost/mpl/assert.hpp>
#include <boost/mpl/not.hpp>
#include <boost/type_traits/is_same.hpp>

/** API to create INSERT queries in a safe way from code, without the risk of
 *  introducing SQL injection vulnerabilities or typos.
 */
class SQLATE_EXPORT SqlInsertQueryBuilder : public SqlQueryBuilderBase
{
public:

    /// Create a new query builder for the given database
    explicit SqlInsertQueryBuilder( const QSqlDatabase &db = QSqlDatabase::database() );

    /// INSERT INTO table ( @p columnName , ... ) VALUES( @p value )
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

    /// INSERT INTO table VALUES ( @p values )...
    void addAllValues( const QVector<QVariant> &values );

    /// INSERT INTO ... DEFAULT VALUES
    void addDefaultValues();

    /// Returns the created query object, when called first, the query object is assembled and prepared
    SqlQuery& query();

private:      
    friend class InsertQueryBuilderTest;
    friend class InsertTest;
    QVector<QPair<QString, QVariant> > m_columns;
    
    QStringList m_columnNames; //holds the column names, used for unit testing
    QVector<QVariant> m_values; //holds the inserted values, used for unit testing    
};

#endif
