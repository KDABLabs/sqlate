/*
    Copyright (C) 2011-2013 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
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
#ifndef SQLDELETEQUERYBUILDER_H
#define SQLDELETEQUERYBUILDER_H

#include "SqlConditionalQueryBuilderBase.h"
#include "sqlate_export.h"

/** API to create DELETE queries in a safe way from code, without the risk of
 *  introducing SQL injection vulnerabilities or typos.
 */
class SQLATE_EXPORT SqlDeleteQueryBuilder : public SqlConditionalQueryBuilderBase
{
public:
    /// Create a new query builder for the given database
    explicit SqlDeleteQueryBuilder( const QSqlDatabase &db = QSqlDatabase::database() );

    /**
     * If @p includeSubTables is true all the tables inheriting from the specified
     * table are deleted. If false, only the table is deleted using
     * DELETE <b>ONLY</b> &lt;tableName&gt;
     */
    void setIncludeSubTables( bool includeSubTables  );

    /// Returns the created query object, when called first, the query object is assembled and prepared
    /// The method throws an SqlException if there is an error preparing the query.
    SqlQuery& query();

private:
    friend class DeleteQueryBuilderTest;
    friend class DeleteTest;
    bool m_includeSubTables;
};

#endif
