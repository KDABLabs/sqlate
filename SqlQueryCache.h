/*
* Copyright (C) 2012-2013 Klaralavdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Author: Volker Krause <volker.krause@kdab.com>
*/
#ifndef SQLQUERYCACHE_H
#define SQLQUERYCACHE_H

#include "sqlate_export.h"

class QString;
class SqlQuery;

/**
 * A per-connection cache prepared query cache.
 */
namespace SqlQueryCache
{
    /// Check whether the query @p queryStatement is cached already.
    SQLATE_EXPORT bool contains( const QString& dbConnectionName, const QString& queryStatement );

    /// Returns the cached (and prepared) query for @p queryStatement.
    SQLATE_EXPORT SqlQuery query( const QString& dbConnectionName, const QString& queryStatement );

    /// Insert @p query into the cache for @p queryStatement.
    SQLATE_EXPORT void insert( const QString& dbConnectionName, const QString& queryStatement, const SqlQuery& query );

    /// Clears the cache, must be called whenever the corresponding database connection is dropped.
    SQLATE_EXPORT void clear();

    /// Enables/disables the query cache. This can be used to temporarily disable caching while changing the db layout.
    SQLATE_EXPORT void setEnabled( bool enable );
}

#endif
