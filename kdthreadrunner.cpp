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


/*!
  \class KDThreadRunner:
  \ingroup core
  \brief Convenience class for starting QThreads that use signals/slots
  \since_c 2.2

  Unexpected behavior occurs when using slots or timers in a QThread subclass
  (since the QThread, as a QObject, belongs to the main thread).
  So better forget that QThread derives from QObject,
  and not use signals and slots inside a thread subclass.

  To solve this problem, you can use KDThreadRunner<T>.
  KDThreadRunner represents a thread (using QThread).
  The implementation of the thread should not be done in a subclass of
  QThread however, but in a separate object of type T.

  T should be a QObject subclass, it will be created inside the thread
  (so you can do all the initializations in the constructor)
  and the thread will have an event loop so for instance timers will work.
  To stop the thread, call quit() on the KDThreadRunner (from the outside),
  or thread()->quit() from the inside of the T class.

  Your constructor for the class T can be defined to take a QObject* parameter
  or a KDThreadRunner<T>* parameter, which will be set to the threadrunner instance,
  but in any case, the constructor should not call ": QObject(parent)",
  it should be an object without a parent. It will be deleted when the
  thread quits. The parameter is simply the threadrunner instance, it can be ignored
  if T doesn't need it.
*/

/*!
  \fn KDThreadRunner::KDThreadRunner( QObject * parent )

  Constructor. \a parent is passed to the base class constructor.
*/

/*!
  \fn KDThreadRunner::startThread( Priority prio )

  Call this function if you need access to the instance of T
  (otherwise call QThread::start()). \a prio is passed to
  QThread::start().

  \note If the payload class quits the eventloop very quickly,
  startThread() might return a pointer to an already deleted object.

  \sa QThread::start, QThread::Priority
*/


#ifdef KDTOOLSCORE_UNITTESTS

#include <QCoreApplication>
#include <QPointer>
#include <QSemaphore>
#include <KDUnitTest/Test>

QSemaphore slotCalledSemaphore;

// these don't need mutex protection, since they are correctly
// sequenced by thread starts and joins:
static bool instanceTestFailedInCtor = false;
static bool instanceTestFailedInSlot = false;
static bool threadAffinityTestFailedInCtor = false;
static bool threadAffinityTestFailedInSlot = false;
static bool threadAffinityTestFailedInReceiver = false;

class TestThreadImpl : public QObject
{
    Q_OBJECT
public:
    explicit TestThreadImpl(QObject* runner)
        : QObject() // no parent!
    {
        if ( thread() != runner )
            threadAffinityTestFailedInCtor = true;
        if ( thread() == QCoreApplication::instance()->thread() )
            instanceTestFailedInCtor = true;
    }

Q_SIGNALS:
    void slotCalled();

private Q_SLOTS:
    void slotInThread()
    {
        if ( QThread::currentThread() == thread() ) // so that the test would fail also in release mode
            if ( QThread::currentThread() != QCoreApplication::instance()->thread() )
                emit slotCalled();
            else
                instanceTestFailedInSlot = true;
        else
            threadAffinityTestFailedInSlot = true;
        slotCalledSemaphore.release();
        thread()->quit();
    }
};

class Emitter : public QObject
{
    Q_OBJECT
public:
    explicit Emitter( QObject * p=0 )
        : QObject( p )
    {}
    void emitSignal() { emit theSignal(); }

Q_SIGNALS:
    void theSignal();
};

class Receiver : public QObject
{
    Q_OBJECT
public:
    explicit Receiver( QObject * p=0 )
        : QObject( p ),
          m_slotCalledCount( 0 )
    {
    }
    
    int slotCalledCount() const 
    { 
        return m_slotCalledCount;
    }

private Q_SLOTS:
    void theSlot()
    {
        if ( thread() == QThread::currentThread() )
            ++m_slotCalledCount;
        else
            threadAffinityTestFailedInReceiver = true;
    }

private:
    int m_slotCalledCount;
};

#include "kdthreadrunner.moc"

KDAB_UNITTEST_SIMPLE( KDThreadRunner, "kdcoretools" ) {

    const int JOIN_TIMEOUT = 10000; // 10s

    KDThreadRunner<TestThreadImpl> threadRunner;
    Emitter emitter; // an object in the gui thread
    Receiver receiver; // this one needs to store the call-count - the impl might be dead after acquire

    {
        instanceTestFailedInSlot = instanceTestFailedInCtor = threadAffinityTestFailedInSlot = threadAffinityTestFailedInCtor = threadAffinityTestFailedInReceiver = false;
        TestThreadImpl* impl = threadRunner.startThread();
        assertNotNull( impl );
        QPointer<QObject> guard(impl);
        assertTrue( threadRunner.isRunning() );
        assertEqual( emitter.thread(), QCoreApplication::instance()->thread() );

        // Check that we can't call startThread while it's running
        TestThreadImpl* impl2 = threadRunner.startThread();
        assertEqual( impl, impl2 );

        bool connectOk = QObject::connect( &emitter, SIGNAL(theSignal()), impl, SLOT(slotInThread()) );
        assertTrue( connectOk );
        connectOk = QObject::connect( impl, SIGNAL(slotCalled()), &receiver, SLOT(theSlot()) );
        assertTrue( connectOk );
        emitter.emitSignal();
        slotCalledSemaphore.acquire();
        assertTrue( threadRunner.wait( JOIN_TIMEOUT ) );
        QCoreApplication::processEvents();
#ifndef Q_WS_MAC // KDTO-187
        assertEqual( receiver.slotCalledCount(), 1 );
#endif
        assertFalse( threadRunner.isRunning() );
        assertNull( guard );

        assertFalse( instanceTestFailedInCtor );
        assertFalse( instanceTestFailedInSlot );
        assertFalse( threadAffinityTestFailedInCtor );
        assertFalse( threadAffinityTestFailedInSlot );
        assertFalse( threadAffinityTestFailedInReceiver );
    }

    // Now do it all again so that we test restarting the thread again (KDTO-178)
    {
        instanceTestFailedInSlot = instanceTestFailedInCtor = threadAffinityTestFailedInSlot = threadAffinityTestFailedInCtor = threadAffinityTestFailedInReceiver = false;
        TestThreadImpl* impl = threadRunner.startThread();
        assertNotNull( impl );
        QPointer<QObject> guard(impl);
        assertTrue( threadRunner.isRunning() );
        assertEqual( emitter.thread(), QCoreApplication::instance()->thread() );

        // Check that we can't call startThread while it's running
        TestThreadImpl* impl2 = threadRunner.startThread();
        assertEqual( impl, impl2 );

        bool connectOk = QObject::connect( &emitter, SIGNAL(theSignal()), impl, SLOT(slotInThread()) );
        assertTrue( connectOk );
        connectOk = QObject::connect( impl, SIGNAL(slotCalled()), &receiver, SLOT(theSlot()) );
        assertTrue( connectOk );
        emitter.emitSignal();
        slotCalledSemaphore.acquire();
        assertTrue( threadRunner.wait( JOIN_TIMEOUT ) );
        QCoreApplication::processEvents();
#ifndef Q_WS_MAC // KDTO-187
        assertEqual( receiver.slotCalledCount(), 2 );
#endif
        assertFalse( threadRunner.isRunning() );
        assertNull( guard );

        assertFalse( instanceTestFailedInCtor );
        assertFalse( instanceTestFailedInSlot );
        assertFalse( threadAffinityTestFailedInCtor );
        assertFalse( threadAffinityTestFailedInSlot );
        assertFalse( threadAffinityTestFailedInReceiver );
    }
}

#endif // KDTOOLSCORE_UNITTESTS
