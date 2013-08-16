/*
    Copyright (C) 2011-2013 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Volker Krause <volker.krause@kdab.com>
        author Andras Mantia <andras.mantia@kdab.com>

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
