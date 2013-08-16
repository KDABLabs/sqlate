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
#ifndef SQLUTILS_H
#define SQLUTILS_H

#include "sqlate_export.h"

#include "SqlSelectQueryBuilder.h"

#include <QSqlField>
#include <QSqlRecord>

class QStringList;
class QString;
class QSqlError;

/**
 * Misc helper functions around SQL/database access.
 */
namespace SqlUtils
{
    /**
     * Strips out comments from an SQL string.
     */
    SQLATE_EXPORT void stripComments(QString &query);

    /**
     * Splits a list of SQL statements provided in one string into a list of statements.
     */
    SQLATE_EXPORT QStringList splitQueries(const QString &queries);

    /**
     * Executes SQL commands from a string.
     */
    SQLATE_EXPORT QString batchExec(const QString& sql, const QString& connectionName/*=QSqlDatabase::defaultConnection*/);

    /**
     * Returns a list of name/type pairs describing the columns in table @p table.
     * @throws SqlExeception in case of a database error.
     */
    template <typename T>
    inline QVector<QPair<QString, QVariant::Type> > columnsOfTable( const T &table, const QSqlDatabase &db = QSqlDatabase::database() )
    {
        SqlSelectQueryBuilder qb( db );
        qb.setTable( table );
        qb.addAllColumns();
        qb.exec();
        const QSqlRecord record = qb.query().record();
        QVector<QPair<QString, QVariant::Type> > columns;
        for (int i = 0; i < record.count(); ++i) {
            const QSqlField field = record.field(i);
            columns.push_back(qMakePair(field.name(), field.type()));
        }
        return columns;
    }

    /** Returns true if the error can be identified as a lock error (cannot obtain lock on a table).
     * Returning false doesn't mean that is is not a lock error, it just means it couldn't
     * be identified as a lock error 100% sure.
     */
    SQLATE_EXPORT bool isLockError(const QSqlError& error);

    /**
     * @brief creates a short unique identifier to identify postgres signals.
     */
    SQLATE_EXPORT QString createIdentifier(const QString& text);

}

#endif
