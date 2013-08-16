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
#include "SchemaUpdater.h"

#include "sql.h"
#include "SqlQuery.h"
#include "SqlUtils.h"

#include <QDebug>

using namespace Sql;

SchemaUpdaterBase::SchemaUpdaterBase (int targetVersion, const QString & pluginName /* = QString()*/ , QWidget* parentWidget /*= 0*/, QObject *parent /*= 0*/) :
    QObject( parent ),
    m_parentWidget(parentWidget),
    m_currentVersion(-1),
    m_targetVersion(targetVersion),
    m_pluginName(pluginName)
{
}

QVector<SchemaUpdaterBase::UpdateInfo> SchemaUpdaterBase::pendingUpdates(const QDir & dir) const
{
    QVector<UpdateInfo> updates;
    updates.reserve(m_targetVersion - m_currentVersion);

    for ( int i = m_currentVersion + 1; i <= m_targetVersion; ++i ) {
        UpdateInfo info;
        info.version = i;
        const QString fileName = QString::fromLatin1("%1.sql").arg(i);
        if (dir.exists(fileName)) {
            info.updateFile = dir.absoluteFilePath(fileName);
            updates.push_back(info);
        }
    }
    return updates;
}

void SchemaUpdaterBase::execUpdate ( const QString& updateFile )
{
    QFile file(updateFile);
    if (file.open(QFile::ReadOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        foreach ( QString statement, SqlUtils::splitQueries(stream.readAll()) ) {
            SqlUtils::stripComments(statement);
            if (statement.isEmpty())
                continue;

            SqlQuery query;
            query.exec(statement);
        }
    } else {
        // coming from a QRC file, always readable...
        qFatal("unable to open %s", qPrintable(updateFile));
    }
}

#include "moc_SchemaUpdater.cpp"
