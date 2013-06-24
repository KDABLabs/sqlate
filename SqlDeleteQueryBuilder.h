/*
* Copyright (C) 2011-2013 Klaralavdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Author: Andras Mantia <andras.mantia@kdab.com>
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
    bool m_includeSubTables;
};

#endif
