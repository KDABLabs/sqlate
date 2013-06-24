/*
* Copyright (C) 2012-2013 Klaralavdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Author: Volker Krause <volker.krause@kdab.com>
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
