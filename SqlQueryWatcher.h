/*
* Copyright (C) 2011 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Author: Andras Mantia <andras.mantia@kdab.com>
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
