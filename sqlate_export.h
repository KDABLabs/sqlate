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
#ifndef SQLATEEXPORT_H
#define SQLATEEXPORT_H

#include <qglobal.h>

# ifdef SQLATE_STATICLIB
#  undef SQLATE_SHAREDLIB
#  define SQLATE_EXPORT
# else
#  ifndef SQLATE_EXPORT
#   ifdef SQLATE_BUILD_SQLATE_LIB
#    define SQLATE_EXPORT Q_DECL_EXPORT
#   else
#    define SQLATE_EXPORT Q_DECL_IMPORT
#   endif
#  endif
# endif

#endif

