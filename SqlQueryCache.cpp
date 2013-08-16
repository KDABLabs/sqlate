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
#include "SqlQueryCache.h"

#include "SqlQuery.h"
#include <QHash>

static QHash<QString, QHash<QString, SqlQuery> > g_queryCache;
static bool g_queryCacheEnabled = true;

bool SqlQueryCache::contains(const QString &dbConnectionName, const QString& queryStatement)
{
    if (!g_queryCacheEnabled)
        return false;
    return g_queryCache.contains(dbConnectionName) && g_queryCache.value(dbConnectionName).contains(queryStatement);
}

SqlQuery SqlQueryCache::query(const QString &dbConnectionName, const QString& queryStatement)
{
    return g_queryCache.value(dbConnectionName).value(queryStatement);
}

void SqlQueryCache::insert(const QString &dbConnectionName, const QString& queryStatement, const SqlQuery& query)
{
    if (g_queryCacheEnabled)
        g_queryCache[dbConnectionName].insert(queryStatement, query);
}

void SqlQueryCache::clear()
{
    g_queryCache.clear();
}

void SqlQueryCache::setEnabled(bool enable)
{
    g_queryCacheEnabled = enable;
    clear();
}
