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
#ifndef SQLTRANSACTION_H
#define SQLTRANSACTION_H

#include "sqlate_export.h"
#include <QSqlDatabase>
#include <QHash>

/**
 * Simple RAII class for transaction handling.
 * @note this is not (yet) thread-safe!
 */
class SQLATE_EXPORT SqlTransaction
{
public:
    /**
     * Starts a transaction on the given database.
     * @throws SqlException if starting the transaction failed
     */
    explicit SqlTransaction( const QSqlDatabase &db = QSqlDatabase::database() );

    /**
     * Rolls back a still ongoing transaction, unless commit() has been called explicitly.
     * @note Does not throw on error.
     */
    ~SqlTransaction();

    /**
     * Commit this transaction.
     * @throws SqlException if committing failed.
     */
    void commit();

    /**
     * Rolls back this transaction.
     * @throws SqlException if rolling back failed
     */
    void rollback();

    /**
     *@Static method that returns the number of transactions being run.
     *That allow us to make sure at least a transaction is running.
     *
     */

    static int transactionsCount();

private:
    Q_DISABLE_COPY( SqlTransaction )
    QSqlDatabase m_db;
    static QHash<QString, int> m_refCounts;
    bool m_disarmed;
};

#endif
