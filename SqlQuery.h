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

/*!
  * This is the class that should be used in place of all QSqlQuery
  * This was we can keep a check on all queries and authenticate
  * appropriately.
  *
  * WARNING: Do NOT cast a SqlQuery to QSqlQuery and call
  * prepare or exec on it, as it will not call the SqlQuery variants!
  */

#ifndef SQLQUERY_H
#define SQLQUERY_H

#include "sqlate_export.h"

#include <QSqlQuery>

class SQLATE_EXPORT SqlQuery : public QSqlQuery
{
public:
    /// @todo Really we should be abstracting all this
    // So this class decides which database to use, i.e. the lookup or main etc
    // When we implement this, move these members to private to pick up where
    // they are used
    SqlQuery (const QString &query = QString(), const QSqlDatabase& db = QSqlDatabase::database() );
    SqlQuery (const QSqlDatabase& db );
    SqlQuery (const SqlQuery & other );

    ~SqlQuery();

    //NOTE: not that nice they overload non-virtual methods, so beware when casting a SqlQuery to QSqlQuery
    // In short: do not cast.
    void exec();
    void exec( const QString &query );
    void prepare( const QString &query );

    /**
     * @brief Prepare the query without checking if the connection to the database is alive.
     *  This should not be called from anywhere, but the SqlQueryManager::checkDbIsAlive to avoid
     *  multiple checking.
     * @param query the query to prepare
     * @return bool true if succeed, false if there was some error. See QSqlQuery::prepare.
     **/
    bool prepareWithoutCheck( const QString &query );

    QString connectionName() const { return m_connectionName; }

    SqlQuery& operator=(const SqlQuery& other);

private:
    QSqlDatabase m_db;
    QString m_connectionName;
};

#endif
