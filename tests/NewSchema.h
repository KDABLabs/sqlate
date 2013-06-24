#ifndef SCHEMAUPDATETEST_NEW_SCHEME_H
#define SCHEMAUPDATETEST_NEW_SCHEME_H

#include "SqlSchema_p.h"

#include <QUuid>
#include <QVariant>

#define NO_EXPORT

using namespace Sql;

namespace NewSql {

TABLE( Version, NO_EXPORT ) {
    SQL_NAME( "tblVersion" );
    COLUMN( version, int, NotNull );
    typedef boost::mpl::vector<versionType> columns;
};

TABLE( Existing, NO_EXPORT ) {
    SQL_NAME( "tblExisting" );
    COLUMN( id, QUuid, PrimaryKey );
    COLUMN( column1, QString, NotNull, 128 );  // <-- column changed constraint to NotNull
    COLUMN( column2, int, Null );              // <-- new column
    typedef boost::mpl::vector<idType, column1Type, column2Type> columns;
};

TABLE( New, NO_EXPORT ) {
    SQL_NAME( "tblNew" );
    COLUMN( id, QUuid, PrimaryKey );
    COLUMN( column1, QString, Null, 42 );
    typedef boost::mpl::vector<idType, column1Type> columns;
};


#define NEWSCHEMA (Version)(Existing)(New)
DECLARE_SCHEMA( NewSchema, NEWSCHEMA );

}

#endif
