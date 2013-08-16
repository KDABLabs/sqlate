/*
    Copyright (C) 2011-2013 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
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

#ifndef SQLQUERYWATCHER_H
#define SQLQUERYWATCHER_H

#include "sqlate_export.h"

#include <QThread>

class QTimer;
class QProcess;

/** The actual watcher class, instantiated from a KDThreadRunner thread */
class SQLATE_EXPORT SqlQueryWatcherHelper : public QObject {
Q_OBJECT
public:
    SqlQueryWatcherHelper(QObject* runner);
    virtual ~SqlQueryWatcherHelper();

public Q_SLOTS:
    void quit();

private Q_SLOTS:
    void timerExpired();
    void dialogHidden();

Q_SIGNALS:
     void endWatcher();   
    
private:
    QTimer *m_timer;
    QProcess *m_process;
};

#endif 
