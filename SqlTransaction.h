/*
* Copyright (C) 2011-2013 Klaralavdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Author: Volker Krause <volker.krause@kdab.com>
*         Andras Mantia <andras.mantia@kdab.com>
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
