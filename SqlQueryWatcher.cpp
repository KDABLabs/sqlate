/*
* Copyright (C) 2011 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Author: Andras Mantia <andras.mantia@kdab.com>
*/

#include "SqlQueryWatcher.h"

#include <QDebug>
#include <QTimer>
#include <QEventLoop>
#include <QProcess>
#include <QMutex>

#ifdef Q_OS_WIN
#include <process.h>
#else
#include <unistd.h>
#endif

#define SQL_NETWORK_TIMEOUT 30 //define how many seconds do we wait for a query finishes until the network error dialog is shown

QMutex mutex;
static bool s_dialogVisible = false;

SqlQueryWatcherHelper::SqlQueryWatcherHelper(QObject* runner):QObject(0)
{
//     qDebug() << Q_FUNC_INFO << this;
    m_process = 0;
    m_timer = new QTimer();
    m_timer->setSingleShot(true);
    connect( m_timer, SIGNAL(timeout()), SLOT(timerExpired()) );
    m_timer->start( SQL_NETWORK_TIMEOUT*1000 );
}

SqlQueryWatcherHelper::~SqlQueryWatcherHelper()
{
//     qDebug() << Q_FUNC_INFO << this;
    delete m_timer;
    if ( m_process ) {
        m_process->terminate();
        m_process->kill();
        m_process->waitForFinished();
    }
    emit endWatcher();
}

void SqlQueryWatcherHelper::dialogHidden()
{
    QMutexLocker locker(&mutex);
    s_dialogVisible = false;
}


void SqlQueryWatcherHelper::timerExpired()
{
    QMutexLocker locker(&mutex);
    if ( s_dialogVisible )
        return;
    s_dialogVisible = true;
    qDebug() << Q_FUNC_INFO << this;
    m_process = new QProcess(this);
    connect( m_process, SIGNAL(finished(int)), this, SLOT(dialogHidden()) );
    QStringList args;
    args << QLatin1String("-pid");
#ifdef Q_OS_WIN
    args << QString::number( _getpid() );
#else
    args << QString::number( ::getpid() );
#endif

    m_process->start( QLatin1String("queryWatcherHelper"), args );
    m_process->waitForStarted();
    if ( m_process->state() != QProcess::Running ) { //try to start from the build dir
        m_process->start( QLatin1String("src/helpers/queryWatcherHelper"), args );
        m_process->waitForStarted();
        if ( m_process->state() != QProcess::Running ) { //try to start from the build dir
            qWarning() << "Timer expired, query was too slow! Possible network error detected.";
        }
    }
}


void SqlQueryWatcherHelper::quit()
{
//     qDebug() << Q_FUNC_INFO;
    thread()->quit();
//     metaObject()->invokeMethod(thread(), "quit", Qt::DirectConnection);
}

#include "moc_SqlQueryWatcher.cpp"
