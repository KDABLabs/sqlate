#ifndef TESTSCHEMA_H
#define TESTSCHEMA_H

#include "SqlSchema_p.h"

#include <QDateTime>

namespace Sql {

#define TEST_EXPORT

TABLE( Version, TEST_EXPORT ) {
    ADMIN_GROUP("sqladmins")
    USER_GROUP("sqlusers")
    ONLY_USER_SELECT
    SQL_NAME( "tblVersion" );
    COLUMN( version, int, NotNull );
    typedef boost::mpl::vector<versionType> columns;
};

LOOKUP_TABLE( Prefix, TEST_EXPORT ) {
    ADMIN_GROUP("sqladmins")
    USER_GROUP("sqlusers")
    SQL_NAME( "lutPrefix" );
};

LOOKUP_TABLE( PersonRoles, TEST_EXPORT ) {
    ADMIN_GROUP("sqladmins")
    USER_GROUP("sqlusers")
    SQL_NAME( "lutPersonRoles" );
    COLUMN( reportType, int, Default );
    typedef append<baseColumns, boost::mpl::vector<reportTypeType> >::type columns;
};

LOOKUP_TABLE( PersonSubRoles, TEST_EXPORT ) {
    ADMIN_GROUP("sqladmins")
    USER_GROUP("sqlusers")
    SQL_NAME( "lutPersonSubRoles" );
    FOREIGN_KEY( PersonRole, PersonRoles, id, NotNull );
    typedef append<baseColumns, boost::mpl::vector<PersonRoleType> >::type columns;
};

LOOKUP_TABLE( PersonGrades, TEST_EXPORT ) {
    ADMIN_GROUP("sqladmins")
    USER_GROUP("sqlusers")
    SQL_NAME( "lutPersonGrades" );
    FOREIGN_KEY( PersonRole, PersonRoles, id, NotNull|OnDeleteRestrict );
    typedef append<baseColumns, boost::mpl::vector<PersonRoleType> >::type columns;
};
TABLE( Person, TEST_EXPORT ) {
    ADMIN_GROUP("sqladmins")
    USER_GROUP("sqlusers")
    ONLY_USER_SELECT
    SQL_NAME( "tblPerson" );
    COLUMN( id, QUuid, PrimaryKey );
    FOREIGN_KEY( fk_lutPrefix_id, Prefix, id, NotNull|OnDeleteRestrict );
    COLUMN( PersonForename, QString, Null, 128 );
    COLUMN( PersonSurname, QString, Null, 128 );
    COLUMN(PersonSuffix, QString, Null, 128);
    FOREIGN_KEY( PersonGrade, PersonGrades, id, NotNull|OnDeleteRestrict );
    COLUMN( HireRights, bool, Null );
    COLUMN( PersonActive, bool, Null );
    COLUMN( UserName, QString, Unique, 255 );
    COLUMN( Hired, QDateTime, Null );
    typedef boost::mpl::vector<idType, fk_lutPrefix_idType, PersonForenameType, PersonSurnameType, PersonSuffixType, PersonGradeType, PersonActiveType, HireRightsType, UserNameType, HiredType> columns;
};


UNIQUE_RELATION( PersonSubRolesRelation, Person, id, PersonSubRoles, id, TEST_EXPORT ) {
    ADMIN_GROUP("sqladmins")
    USER_GROUP("sqlusers")
    ONLY_USER_SELECT
    SQL_NAME( "rltPersonSubRoles" );
    COLUMN_ALIAS( left, fk_tblPerson_id );
    COLUMN_ALIAS( right, fk_lutPersonSubRoles_id );
};


LOOKUP_TABLE( Directorates, TEST_EXPORT ) {
    ADMIN_GROUP("sqladmins")
    USER_GROUP("sqlusers")
    SQL_NAME( "lutDirectorates" );
};

LOOKUP_TABLE( SubDirectorates, TEST_EXPORT ) {
    ADMIN_GROUP("sqladmins")
    USER_GROUP("sqlusers")
    SQL_NAME( "lutSubDirectorates" );
    FOREIGN_KEY( Directorate, Directorates, id, NotNull );
    typedef append<baseColumns, boost::mpl::vector<DirectorateType> >::type columns;
};

UNIQUE_RELATION( PersonSubDirectoratesRelation, Person, id, SubDirectorates, id, TEST_EXPORT ) {
    ADMIN_GROUP("sqladmins")
    USER_GROUP("sqlusers")
    ONLY_USER_SELECT
    SQL_NAME( "rltPersonSubDirectorates" );
    COLUMN_ALIAS( left, fk_tblPerson_id );
    COLUMN_ALIAS( right, fk_lutSubDirectorates_id );
};

UNIQUE_RECURSIVE_RELATION( PersonBossRelation, Person, id, TEST_EXPORT ) {
    ADMIN_GROUP("sqladmins")
    USER_GROUP("sqlusers")
    ONLY_USER_SELECT
    SQL_NAME( "rltPersonBossRelation" );
    COLUMN( expires, QDateTime, Null );
    COLUMN_ALIAS( left, fk_tblPerson_id );
    COLUMN_ALIAS( right, fk_tblPerson_id_link );
    typedef append<baseColumns, boost::mpl::vector<expiresType> >::type columns;
};

TABLE( WorkTask, TEST_EXPORT ) {
    ADMIN_GROUP("sqladmins")
    USER_GROUP("sqlusers")
    NO_USER_DELETE
    SQL_NAME( "tblWorkTask" );
    COLUMN( ealStart, QDateTime, NotNull );
    COLUMN( ealEnd, QDateTime, Null );
    COLUMN( foreign_id, QUuid, NotNull );
    typedef boost::mpl::vector<ealStartType, ealEndType, foreign_idType> columns;
    typedef append<baseConstraints, boost::mpl::vector<UniqueConstraint<boost::mpl::vector<foreign_idType, ealEndType> > > >::type constraints;
};
TABLE( Workplace, TEST_EXPORT ) {
    ADMIN_GROUP("sqladmins")
    USER_GROUP("sqlusers")
    ONLY_USER_SELECT
    SQL_NAME( "tblWorkplace" );
    COLUMN( id, QUuid, PrimaryKey | Notify );
    COLUMN( itemorder, int, NotNull );
    COLUMN( short_desc, QString, Null, 32 );
    COLUMN( description, QString, Null, 255 );
    COLUMN( contact_tel, QString, Null, 128 );
    COLUMN( contact_fax, QString, Null, 128 );
    typedef boost::mpl::vector<idType,  itemorderType, short_descType, descriptionType, contact_telType, contact_faxType> columns;
};

TABLE( Report, TEST_EXPORT ) {
    ADMIN_GROUP("sqladmins")
    USER_GROUP("sqlusers")
    NO_USER_DELETE
    SQL_NAME( "tblReport" );
    COLUMN( id, QUuid, PrimaryKey );
    COLUMN( ts, QDateTime, NotNull );
    COLUMN( txt, QString, Null );
    typedef boost::mpl::vector<idType, tsType, txtType> columns;
};

#define SCHEMA (Version)(Prefix)(PersonRoles)(PersonSubRoles)(PersonGrades)(Person)(PersonSubRolesRelation)(Directorates)(SubDirectorates)(PersonSubDirectoratesRelation)(PersonBossRelation)(WorkTask)(Workplace)(Report)
DECLARE_SCHEMA( SQLateTestSchema, SCHEMA );
}

#endif
