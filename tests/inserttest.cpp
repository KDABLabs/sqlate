#define QUERYBUILDER_UNITTEST
#include "testschema.h"
#include "testbase.h"
#include "Sql.h"
#include "SqlInsert.h"

#include <QObject>
#include <QtTest/QtTest>

using namespace Sql;

Q_DECLARE_METATYPE( SqlInsertQueryBuilder )
Q_DECLARE_METATYPE( QVector<QVariant> )

class InsertTest : public TestBase
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() {
        openDbTest();
        createEmptyDb();
    }

    void testInsert_data()
    {
        QTest::addColumn<SqlInsertQueryBuilder>( "qb" );
        QTest::addColumn<QString>( "sql" );
        QTest::addColumn<QVector<QVariant> >( "bindVals" );

        QTest::newRow( "single col, one value" )
                << insert()
                   .into( Person )
                   .columns( Person.PersonSurname << QLatin1String( "Ford" ) ).queryBuilder()
                << "INSERT INTO tblPerson (PersonSurname) VALUES (:0)"
                << (QVector<QVariant>() << QLatin1String( "Ford" ));

        QTest::newRow( "two col, two values" )
                << insert()
                   .into( Person )
                   .columns( Person.PersonForename << QLatin1String( "Ford" ) &
                             Person.PersonSurname << QLatin1String( "Prefect" ) ).queryBuilder()
                << "INSERT INTO tblPerson (PersonForename,PersonSurname) VALUES (:0,:1)"
                << (QVector<QVariant>() << QLatin1String( "Ford" ) << QLatin1String( "Prefect" ));

        QTest::newRow( "two col, default values" )
                << insert()
                   .into( Person )
                   .columns( Person.PersonForename & Person.PersonSurname )
                   .defaultValues().queryBuilder()
                << "INSERT INTO tblPerson (PersonForename,PersonSurname) VALUES (DEFAULT,DEFAULT)"
                << (QVector<QVariant>());

        QTest::newRow( "two col, one default, one not" )
                << insert()
                   .into( Person )
                   .columns( Person.PersonForename << QLatin1String( "Ford" ) & Person.PersonSurname )
                   .queryBuilder()
                << "INSERT INTO tblPerson (PersonForename,PersonSurname) VALUES (:0,DEFAULT)"
                << (QVector<QVariant>() << QLatin1String( "Ford" ));

        QTest::newRow( "two col, bool and datetime" )
                << insert()
                   .into( Person )
                   .columns( Person.HireRights << false & Person.Hired << QDateTime(QDate(2013, 1, 1)) )
                   .queryBuilder()
                << "INSERT INTO tblPerson (HireRights,Hired) VALUES (:0,:1)"
                << (QVector<QVariant>() << false << QDateTime(QDate(2013, 1, 1)));
    }

    void testInsert()
    {
        QFETCH( SqlInsertQueryBuilder, qb );
        QFETCH( QString, sql );
        QFETCH( QVector<QVariant>, bindVals );

        qb.query(); // trigger query assembly
        QCOMPARE( qb.m_queryString, sql );
        QCOMPARE( qb.m_values.values().toVector(), bindVals );
    }
};

QTEST_MAIN( InsertTest )

#include "inserttest.moc"
