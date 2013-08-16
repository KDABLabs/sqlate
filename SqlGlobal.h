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
#ifndef SQLATE_GLOBAL_H
#define SQLATE_GLOBAL_H

#include <QDebug>
#include <QUuid>

#include <boost/static_assert.hpp>

#define SQLATE_DEFAULT_SERVER_PORT 5432

#define SQLDEBUG  qDebug() << QString::fromLatin1("%1:%2").arg(QLatin1String( __FILE__ )).arg(__LINE__ ) << QLatin1String( ": " )


namespace Sql {
 /** A memory-layout compatible version of QUuid that can be statically initialized. */
struct StaticUuid {
    uint   data1;
    ushort data2;
    ushort data3;
    uchar  data4[8];
    operator const QUuid &() const { return *reinterpret_cast<const QUuid*>(this); }
};

BOOST_STATIC_ASSERT( sizeof(QUuid) == sizeof(StaticUuid) );
}
#endif
