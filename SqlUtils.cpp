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
#include "SqlUtils.h"
#include "SqlTransaction.h"
#include "SqlExceptions.h"
#include "SqlGlobal.h"

#include <QRegExp>
#include <QString>
#include <QStringList>
#include <QSqlError>

void SqlUtils::stripComments ( QString& query )
{
    QRegExp re(QLatin1String( "--.+([\n\r]{1,2};?|$)" ));
    re.setMinimal(true);
    query.remove( re );
    query = query.trimmed();
}

QStringList SqlUtils::splitQueries ( const QString& queries )
{
    return queries.split(QLatin1String( ";\n" ), QString::SkipEmptyParts);
}

QString SqlUtils::batchExec(const QString& sql,
                            const QString& connectionName/*=QSqlDatabase::defaultConnection*/) {
    QString errorText;
    if ( sql.size() > 0 ) {
        const QStringList stmts = splitQueries(sql);
        QSqlDatabase db = QSqlDatabase::database(connectionName);

        if ( stmts.count() == 0 )
            return QString();

        SqlTransaction transaction( db );

        for (int i=0; i < stmts.count(); i++) {
            QString sqlstmt = stmts.at(i);

            stripComments(sqlstmt);

            if (sqlstmt.size() < 3 ) {
                //SQLDEBUG << "#" << i << ") SKIPPING small statement: " << sqlstmt;
                break;
            }

            //SQLDEBUG << QString("POST # %1 %2").arg(i).arg(sqlstmt);

            QSqlQuery q = db.exec( sqlstmt );
            if (q.lastError().type() != QSqlError::NoError) {
                errorText = QObject::tr("Error in one of the batch statements. %1").arg(q.lastError().text());

                qCritical() << QString::fromLatin1("%1.\nStatement: %2").arg(errorText).arg(sqlstmt);
                return errorText;
            }
        }
        transaction.commit();
        return QString();
    }
    else {
        errorText = QObject::tr("SQL statement passed was empty!!");
        return errorText;
    }
}

bool SqlUtils::isLockError(const QSqlError& error)
{
    if (error.type() == QSqlError::StatementError &&
        ( error.text().contains(QLatin1String("(55P03)")) || //the actual error code, not translated. needs patched QPSQL driver
          error.text().contains(QLatin1String("could not obtain lock")) )
       )
    {
        return true;
    }
    return false;
}

QString SqlUtils::createIdentifier(const QString& text)
{
    uint hash = qHash(text);
    return QString::number(hash, 16);
}
