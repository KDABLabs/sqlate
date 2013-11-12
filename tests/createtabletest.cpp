#include "testschema.h"
#include "Sql.h"

#include <QObject>
#include <QtTest/QtTest>

class CreateTableTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testSingleCreation()
    {
        QString s = QString::fromLatin1( "CREATE TABLE tblPerson (\n"
                                            "id UUID PRIMARY KEY,\n"
                                            "fk_lutPrefix_id UUID NOT NULL REFERENCES lutPrefix (id) ON DELETE RESTRICT,\n"
                                            "PersonForename VARCHAR(128),\n"
                                            "PersonSurname VARCHAR(128),\n"
                                            "PersonSuffix VARCHAR(128),\n"
                                            "fk_lutPersonGrades_id UUID NOT NULL REFERENCES lutPersonGrades (id) ON DELETE RESTRICT,\n"
                                            "PersonActive BOOLEAN,\n"
                                            "HireRights BOOLEAN,\n"
                                            "UserName VARCHAR(255) UNIQUE,\n"
                                            "Hired TIMESTAMP WITH TIME ZONE\n)" );
        QCOMPARE( Sql::createTableStatement( Sql::Person ), s );
    }

    void testStandardLookupTable()
    {
        QString s = QString::fromLatin1( "CREATE TABLE lutDirectorates (\n"
                                            "id UUID PRIMARY KEY,\n"
                                            "short_desc VARCHAR(128),\n"
                                            "description VARCHAR(255)\n)" );
        QCOMPARE( Sql::createTableStatement( Sql::Directorates), s );

    }

    void testExtendedLookupTable()
    {
        QString s = QString::fromLatin1( "CREATE TABLE lutPersonGrades (\n"
                                            "id UUID PRIMARY KEY,\n"
                                            "short_desc VARCHAR(128),\n"
                                            "description VARCHAR(255),\n"
                                            "fk_lutPersonRoles_id UUID NOT NULL REFERENCES lutPersonRoles (id) ON DELETE RESTRICT\n)" );
        QCOMPARE( Sql::createTableStatement( Sql::PersonGrades), s );
    }

    void testStandardRelation()
    {
        QString s = QString::fromLatin1( "CREATE TABLE rltPersonSubRoles (\n"
                                            "fk_tblPerson_id UUID NOT NULL REFERENCES tblPerson (id),\n"
                                            "fk_lutPersonSubRoles_id UUID NOT NULL REFERENCES lutPersonSubRoles (id),\n"
                                            "UNIQUE( fk_tblPerson_id, fk_lutPersonSubRoles_id )\n)" );
        QCOMPARE( Sql::createTableStatement( Sql::PersonSubRolesRelation ), s );
    }

    void testExtendedRecursiveRelation()
    {
        QString s = QString::fromLatin1( "CREATE TABLE rltPersonBossRelation (\n"
                                            "fk_tblPerson_id UUID NOT NULL REFERENCES tblPerson (id),\n"
                                            "fk_tblPerson_id_link UUID NOT NULL REFERENCES tblPerson (id),\n"
                                            "expires TIMESTAMP WITH TIME ZONE,\n"
                                            "UNIQUE( fk_tblPerson_id, fk_tblPerson_id_link )\n)" );
        QCOMPARE( Sql::createTableStatement( Sql::PersonBossRelation ), s );
    }

    void testDefault()
    {
        QString s = QString::fromLatin1( "CREATE TABLE lutPersonRoles (\n"
                                            "id UUID PRIMARY KEY,\n"
                                            "short_desc VARCHAR(128),\n"
                                            "description VARCHAR(255),\n"
                                            "reportType INTEGER DEFAULT 0\n)" );
        QCOMPARE( Sql::createTableStatement( Sql::PersonRoles ), s );
    }

    void testUniqueTableConstraint()
    {
        QString s = QString::fromLatin1( "CREATE TABLE tblWorkTask (\n"
                                            "ealStart TIMESTAMP WITH TIME ZONE NOT NULL,\n"
                                            "ealEnd TIMESTAMP WITH TIME ZONE,\n"
                                            "foreign_id UUID NOT NULL,\n"
                                            "UNIQUE( foreign_id, ealEnd )\n)" );
        QCOMPARE( Sql::createTableStatement( Sql::WorkTask ), s );
    }

    void testAllCreation()
    {
        // mostly to check if that compiles, it finds errrors in incomplete schema declarations
        /*qDebug() <<*/ Sql::createTableStatements<Sql::SQLateTestSchema>();
    }
};

QTEST_MAIN( CreateTableTest )

#include "createtabletest.moc"
