#ifndef SQLTEST_UTILS_H
#define SQLTEST_UTILS_H

#include "SqlCondition.h"
#include "SqlSchema.h" //for QUuid metatype

#include <QMetaType>
#include <QUuid>

#define QL1S(x) QString::fromLatin1(x)

/** Extended variant comparison that also handles QUuid and our SQL dummy types. */
inline bool deepVariantCompare( const QVariant &v1, const QVariant &v2 )
{
    if ( v1.type() != v2.type() )
        return false;
    if ( v1.userType() == qMetaTypeId<QUuid>() )
        return v1.value<QUuid>() == v2.value<QUuid>();
    if ( v1.userType() == qMetaTypeId<SqlNowType>() )
        return v2.userType() == qMetaTypeId<SqlNowType>();
    else
        return v1 == v2;
}

#endif
