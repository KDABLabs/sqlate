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
#ifndef SQL_H
#define SQL_H

/**
 * @file Sql.h
 * Convenience include for all the SQL template stuff
 * Preferably include this file instead of any of the other ones directly,
 * since this one cleans up a bunch of otherwise leaked macros.
 */

/**
 * @namespace Sql
 * Sql access classes and templates, including the database schema definition.
 */

#include "SqlSchema.h"
#include "SqlCreateTable.h"
#include "SqlSelect.h"

// macro cleanup
#undef SQL_NAME
#undef COLUMN
#undef COLUMN_ALIAS
#undef FOREIGN_KEY
#undef TABLE
#undef LOOKUP_TABLE
#undef RELATION
#undef RECURSIVE_RELATION
#undef DECLARE_SCHEMA
#undef DEFINE_SCHEMA
#undef DECLARE_SCHEMA_MAKE_TYPE
#undef DEFINE_SCHEMA_MAKE_DEF

#endif
