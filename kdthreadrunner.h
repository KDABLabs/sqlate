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
