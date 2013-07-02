#ifndef SCHEMAUPDATETEST_OLD_SCHEME_H
#define SCHEMAUPDATETEST_OLD_SCHEME_H

#include "SqlSchema_p.h"

#include <QUuid>
#include <QVariant>

#define NO_EXPORT

using namespace Sql;

namespace OldSql {

TABLE( Version, NO_EXPORT ) {
    ADMIN_GROUP("sqladmins")
    USER_GROUP("sqlusers")
    SQL_NAME( "tblVersion" );
    COLUMN( version, int, NotNull );
    typedef boost::mpl::vector<versionType> columns;
};

TABLE( Existing, NO_EXPORT ) {
    ADMIN_GROUP("sqladmins")
    USER_GROUP("sqlusers")
    SQL_NAME( "tblExisting" );
    COLUMN( id, QUuid, PrimaryKey );
    COLUMN( column1, QString, Null, 128 );
    typedef boost::mpl::vector<idType, column1Type> columns;
};

#define OLDSCHEMA (Version)(Existing)
DECLARE_SCHEMA( OldSchema, OLDSCHEMA );

}

#endif
