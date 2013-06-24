/*
* Copyright (C) 2011-2013 Klaralavdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Author: Volker Krause <volker.krause@kdab.com>
*         Andras Mantia <andras.mantia@kdab.com>
*/
#include "SqlCreateTable.h"

namespace Sql {
namespace detail {

QString sqlType( wrap<QString>, int size )
{
    if ( size > 0 )
        return QLatin1Literal( "VARCHAR(" ) % QString::number( size ) % QLatin1Literal( ")" );
    return QLatin1String( "TEXT" );
}

QString sqlType( wrap<bool>, int size )
{
    Q_ASSERT( size == -1 );
    return QLatin1String( "BOOLEAN" );
}

QString sqlType( wrap<QUuid>, int size )
{
    Q_ASSERT( size == -1 );
    return QLatin1String( "UUID" );
}

QString sqlType( wrap<int>, int size )
{
    Q_UNUSED( size );
    /// \todo in theory there are different sized integer types
    return QLatin1String( "INTEGER" );
}

QString sqlType( wrap<QDateTime>, int size )
{
    Q_ASSERT( size == -1 );
    return QLatin1String( "TIMESTAMP WITH TIME ZONE" );
}

QString sqlType( wrap<QTime>, int size )
{
    Q_ASSERT( size == -1 );
    return QLatin1String( "TIME" );
}

QString sqlType( wrap<QDate>, int size ) {
    Q_ASSERT( size == -1 );
    return QLatin1String( "DATE" );
}

QString sqlType( wrap<QByteArray>, int size )
{
    Q_UNUSED( size );
    // ???
    return QLatin1String( "BYTEA" );
}

QString sqlType(Sql::detail::wrap< float > , int size)
{
    Q_UNUSED(size);
    return QLatin1String("REAL");
}


} // namespace detail

} // namespace Sql
