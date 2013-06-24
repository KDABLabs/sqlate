/*
* Copyright (C) 2011-2013 Klaralavdalens Datakonsult AB, a KDAB Group company, info@kdab.com
* Author: Volker Krause <volker.krause@kdab.com>
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
