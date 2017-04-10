#define QUERYBUILDER_UNITTEST
#include "testschema.h"
#include "testbase.h"
#include "Sql.h"
#include "SqlDelete.h"

#include <QObject>
#include <QtTest/QtTest>

using namespace Sql;

Q_DECLARE_METATYPE( SqlDeleteQueryBuilder )
Q_DECLARE_METATYPE( QVector<QVariant> )

class DeleteTest : public TestBase
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() {
        openDbTest();
        createEmptyDb();
    }

    void testDelete_data()
    {
        QTest::addColumn<SqlDeleteQueryBuilder>( "qb" );
        QTest::addColumn<QString>( "sql" );
        QTest::addColumn<QVector<QVariant> >( "bindVals" );

        QTest::newRow( "delete rows, one condition" )
                << del()
                   .from( Person )
                   .where( Person.PersonSurname == QLatin1String( "Ford" ) ).queryBuilder()
                << "DELETE FROM tblPerson WHERE tblPerson.PersonSurname = :0"
                << (QVector<QVariant>() << QLatin1String( "Ford" ));

        QTest::newRow( "delete rows, multiple conditions" )
                << del()
                   .from( Person )
                   .where( Person.PersonSurname == QLatin1String( "Ford" ) && Person.PersonForename == QLatin1String( "Gerald" )).queryBuilder()
                << "DELETE FROM tblPerson WHERE (tblPerson.PersonSurname = :0 AND tblPerson.PersonForename = :1)"
                << (QVector<QVariant>() << QLatin1String( "Ford" ) << QLatin1String("Gerald"));

        QTest::newRow( "delete all rows" )
                << del()
                   .from( Person ).queryBuilder()
                << "DELETE FROM tblPerson"
                << QVector<QVariant>();
    }

    void testDelete()
    {
        QFETCH( SqlDeleteQueryBuilder, qb );
        QFETCH( QString, sql );
        QFETCH( QVector<QVariant>, bindVals );

        qb.query(); // trigger query assembly
        QCOMPARE( qb.m_queryString, sql );
        QCOMPARE( qb.m_bindValues, bindVals );
    }
};

QTEST_MAIN( DeleteTest )

#include "deletetest.moc"
