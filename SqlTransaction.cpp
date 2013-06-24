/*
* Copyright (C) 2011-2013 Klaralavdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Author: Volker Krause <volker.krause@kdab.com>
*         Andras Mantia <andras.mantia@kdab.com>
*/
#include "SqlTransaction.h"
#include "SqlExceptions.h"
#include "SqlQueryManager.h"

#include <QSqlDriver>

QHash<QString, int> SqlTransaction::m_refCounts;

SqlTransaction::SqlTransaction(const QSqlDatabase& db) : m_db( db ), m_disarmed( false )
{
    SqlQueryManager::instance()->checkDbIsAlive(m_db);
    Q_ASSERT( db.driver()->hasFeature( QSqlDriver::Transactions ) );
    if ( m_refCounts.value( m_db.connectionName() ) == 0 && !m_db.transaction() ) {
        SqlQueryManager::instance()->checkDbIsAlive(m_db); //double check is needed, as Qt might not set m_db.isOpen() to false after connection loss if no queries were run meantime.
        if ( m_refCounts.value( m_db.connectionName() ) == 0 && !m_db.transaction() )
            throw SqlException( m_db.lastError() );
    }
    m_refCounts[m_db.connectionName()]++;
}

SqlTransaction::~SqlTransaction()
{
    if ( m_disarmed )
        return;
    SqlQueryManager::instance()->checkDbIsAlive(m_db);
    if ( m_refCounts.value( m_db.connectionName() ) == 1 )
        m_db.rollback();
    m_refCounts[m_db.connectionName()]--;
}

void SqlTransaction::commit()
{
    SqlQueryManager::instance()->checkDbIsAlive(m_db);
    Q_ASSERT( m_refCounts.value( m_db.connectionName() ) > 0 );
    if ( m_refCounts.value( m_db.connectionName() ) == 1 && !m_db.commit() ) {
        SqlQueryManager::instance()->checkDbIsAlive(m_db);  //double check is needed, as Qt might not set m_db.isOpen() to false after connection loss if no queries were run meantime.
        if ( m_refCounts.value( m_db.connectionName() ) == 1 && !m_db.commit() )
            throw SqlException( m_db.lastError() );
    }
    m_refCounts[ m_db.connectionName() ]--;
    m_disarmed = true;
}

void SqlTransaction::rollback()
{
    SqlQueryManager::instance()->checkDbIsAlive(m_db);
    Q_ASSERT( m_refCounts.value( m_db.connectionName() ) > 0 );
    m_refCounts[ m_db.connectionName() ]--;
    m_disarmed = true;
    if ( m_refCounts.value( m_db.connectionName() ) == 0 && !m_db.rollback() ) {
        SqlQueryManager::instance()->checkDbIsAlive(m_db);  //double check is needed, as Qt might not set m_db.isOpen() to false after connection loss if no queries were run meantime.
        if ( m_refCounts.value( m_db.connectionName() ) == 0 && !m_db.rollback() )
            throw SqlException( m_db.lastError() );
    }
}

int SqlTransaction::transactionsCount()
{
    return m_refCounts.size();
}
