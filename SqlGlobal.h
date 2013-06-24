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
