/*
    Copyright (C) 2011-2013 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Volker Krause <volker.krause@kdab.com>

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
