/*
 * Copyright (C) 2012 Klarälvdalens Datakonsult AB, a KDAB Group company, info@kdab.com
 * Author: Volker Krause <volker.krause@kdab.com>
 */

#ifndef POSTGRESSCHEMA_H
#define POSTGRESSCHEMA_H

#include "SqlSchema_p.h"
#include "sqlate_export.h"

#include <QDateTime>

/**
 * Schema definition for the PostgreSQL system tables to the extend we need to access them
 * @note Do not use this in combination with the create table query builder!
 */
namespace Sql {

TABLE( PgUser, SQLATE_EXPORT ) {
    SQL_NAME( "pg_user" );
    COLUMN( usename, QString, Null );
    COLUMN( usesysid, int, Null );
    COLUMN( usecreatedb, bool, Null );
    COLUMN( usesuper, bool, Null );
    COLUMN( usecatupd, bool, Null );
    COLUMN( userepl, bool, Null );
    COLUMN( passwd, QString, Null );
    COLUMN( valuntil, QDateTime, Null );
    typedef boost::mpl::vector<usenameType, usesysidType, usecreatedbType, usesuperType, usecatupdType, usereplType, passwdType, valuntilType> columns;
};

TABLE( PgGroup, SQLATE_EXPORT ) {
    SQL_NAME( "pg_group" );
    COLUMN( groname, QString, Null );
    COLUMN( grosysid, int, Null );
    typedef boost::mpl::vector<gronameType, grosysidType> columns;
};

TABLE( PgAuthMembers, SQLATE_EXPORT ) {
    SQL_NAME( "pg_auth_members" );
    COLUMN( roleid, int, NotNull );
    COLUMN( member, int, NotNull );
    COLUMN( grantor, int, NotNull );
    COLUMN( admin_option, bool, NotNull );
    typedef boost::mpl::vector<roleidType, memberType, grantorType, admin_optionType> columns;
};

#define POSTGRES_SCHEMA (PgUser)(PgGroup)(PgAuthMembers)

DECLARE_SCHEMA( PostgresSchema, POSTGRES_SCHEMA );

}

#endif