#ifndef __KDTOOLSCORE_KDTHREADRUNNER_H__
#define __KDTOOLSCORE_KDTHREADRUNNER_H__

#include <QMutexLocker>
#include <QtCore/QThread>

#include "sqlate_export.h"

class SQLATE_EXPORT KDThreadRunnerBase : public QThread
{
public:
    ~KDThreadRunnerBase();

protected:
    explicit KDThreadRunnerBase( QObject* parent = 0 );
    void doStart( Priority prio );
    void doExec();

    void setImpl( QObject* );
    QObject* impl() const;

    QMutex* internalStartThreadMutex();

private:
    class Private;
    QScopedPointer<Private> d;
};

template <class T>
class SQLATE_EXPORT KDThreadRunner
#ifdef DOXYGEN_RUN
 : public QThread
#else
 : public KDThreadRunnerBase
#endif
{
public:
    explicit KDThreadRunner( QObject * p=0 )
        : KDThreadRunnerBase( p ) {}

    T * startThread( Priority prio = InheritPriority ) {
        QMutexLocker locker( this->internalStartThreadMutex() );
        if ( !this->impl() ) {
            this->doStart( prio );
        }
        return static_cast<T*>( this->impl() );
    }

protected:
    /*! \reimp */ void run()
    {
        // impl is created in the thread so that m_impl->thread()==this
        T t(this);
        this->setImpl( &t );
        this->doExec();
        this->setImpl( 0 );
    }
};

#endif
