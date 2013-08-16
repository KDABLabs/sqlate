#include "kdthreadrunner.h"

#include <QSemaphore>

class KDThreadRunnerBase::Private
{
public:
    Private()
        : m_impl( 0 )
    {
    }

    QObject* m_impl;
    QSemaphore m_waitStarted;
    QMutex m_startThreadMutex;
};

KDThreadRunnerBase::KDThreadRunnerBase( QObject * p )
    : QThread( p )
{
}

KDThreadRunnerBase::~KDThreadRunnerBase()
{
}

void KDThreadRunnerBase::doStart( Priority prio )
{
    start( prio );
    d->m_waitStarted.acquire(); // wait for T to be created by run()
}

void KDThreadRunnerBase::doExec()
{
    // Tell startThread that we have created T
    d->m_waitStarted.release();

    exec();
}

void KDThreadRunnerBase::setImpl( QObject * i )
{
    d->m_impl = i;
}

QObject* KDThreadRunnerBase::impl() const
{
    return d->m_impl;
}

QMutex* KDThreadRunnerBase::internalStartThreadMutex()
{
    return & d->m_startThreadMutex;
}

#include "kdthreadrunner.moc"
