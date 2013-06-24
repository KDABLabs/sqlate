#define QUERYBUILDER_UNITTEST
#include "testschema.h"
#include "testbase.h"
#include "Sql.h"
#include "SqlSelect.h"

#include <QObject>
#include <QtTest/QtTest>

using namespace Sql;

Q_DECLARE_METATYPE( SqlSelectQueryBuilder )
Q_DECLARE_METATYPE( QVector<QVariant> )

class SelectTest : public TestBase
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() {
      openDbTest();
      createEmptyDb();
    }
    
    void testSelect_data()
    {
        QTest::addColumn<SqlSelectQueryBuilder>( "qb" );
        QTest::addColumn<QString>( "sql" );
        QTest::addColumn<QVector<QVariant> >( "bindVals" );

        QTest::newRow( "single col, no cond" ) << select( Person.id ).from( Person ).queryBuilder() << "SELECT tblPerson.id FROM tblPerson" << QVector<QVariant>();

        QTest::newRow( "single condition" )
            << select( Person.id ).from( Person ).where( Person.HireRights == true ).queryBuilder()
            << "SELECT tblPerson.id FROM tblPerson WHERE tblPerson.HireRights = :0"
            << (QVector<QVariant>() << true);

#ifdef _BullseyeCoverage
#pragma BullseyeCoverage off
#endif
        QTest::newRow( "double condition" )
            << select( Person.id ).from( Person ).where( Person.HireRights == true || Person.PersonForename == Person.PersonSurname ).queryBuilder()
            << "SELECT tblPerson.id FROM tblPerson WHERE (tblPerson.HireRights = :0 OR tblPerson.PersonForename = tblPerson.PersonSurname)"
            << (QVector<QVariant>() << true);

        QTest::newRow( "multi col, multi cond" )
            << select(
                Person.id,
                Person.PersonForename,
                Person.PersonSurname
            ).from( Person )
            .where( Person.PersonSurname == Person.PersonForename && Person.PersonActive == true ).queryBuilder()
            << "SELECT tblPerson.id, tblPerson.PersonForename, tblPerson.PersonSurname "
               "FROM tblPerson WHERE (tblPerson.PersonSurname = tblPerson.PersonForename AND tblPerson.PersonActive = :0)"
            << (QVector<QVariant>() << 1);
#ifdef _BullseyeCoverage
#pragma BullseyeCoverage on
#endif

         QTest::newRow( "single join" )
            << select( Person.id, PersonGrades.description ).from( Person ).innerJoin( PersonGrades, Person.PersonGrade == PersonGrades.id ).queryBuilder()
            << "SELECT tblPerson.id, lutPersonGrades.description FROM tblPerson INNER JOIN lutPersonGrades ON tblPerson.fk_lutPersonGrades_id = lutPersonGrades.id"
            << QVector<QVariant>();

        QTest::newRow( "double join" )
            << select( Person.id ).from( Person ).innerJoin( PersonSubDirectoratesRelation, Person.id == PersonSubDirectoratesRelation.left )
                .innerJoin( SubDirectorates, PersonSubDirectoratesRelation.right == SubDirectorates.id ).queryBuilder()
            << "SELECT tblPerson.id FROM tblPerson INNER JOIN rltPersonSubDirectorates ON tblPerson.id = rltPersonSubDirectorates.fk_tblPerson_id "
               "INNER JOIN lutSubDirectorates ON rltPersonSubDirectorates.fk_lutSubDirectorates_id = lutSubDirectorates.id"
            << QVector<QVariant>();

        QTest::newRow( "double join with condtion" )
            << select( Person.id ).from( Person ).innerJoin( PersonSubDirectoratesRelation, Person.id == PersonSubDirectoratesRelation.left )
                .innerJoin( SubDirectorates, PersonSubDirectoratesRelation.right == SubDirectorates.id )
                .where( SubDirectorates.description == QString::fromLatin1( "foo" ) ).queryBuilder()
            << "SELECT tblPerson.id FROM tblPerson INNER JOIN rltPersonSubDirectorates ON tblPerson.id = rltPersonSubDirectorates.fk_tblPerson_id "
               "INNER JOIN lutSubDirectorates ON rltPersonSubDirectorates.fk_lutSubDirectorates_id = lutSubDirectorates.id "
               "WHERE lutSubDirectorates.description = :0"
            << (QVector<QVariant>() << QLatin1String( "foo" ));

        QTest::newRow( "leftOuterJoin" )
            << select( Person.id, PersonGrades.description ).from( Person ).leftOuterJoin( PersonGrades, Person.PersonGrade == PersonGrades.id ).queryBuilder()
            << "SELECT tblPerson.id, lutPersonGrades.description FROM tblPerson LEFT OUTER JOIN lutPersonGrades ON tblPerson.fk_lutPersonGrades_id = lutPersonGrades.id"
            << QVector<QVariant>();

        QTest::newRow( "single order by, default" )
            << select( Person.id ).from( Person ).orderBy( Person.PersonSurname ).queryBuilder()
            << "SELECT tblPerson.id FROM tblPerson ORDER BY tblPerson.PersonSurname ASC"
            << QVector<QVariant>();

        QTest::newRow( "single order by, desc" )
            << select( Person.id ).from( Person ).orderBy( Person.PersonSurname, Qt::DescendingOrder ).queryBuilder()
            << "SELECT tblPerson.id FROM tblPerson ORDER BY tblPerson.PersonSurname DESC"
            << QVector<QVariant>();

        QTest::newRow( "double order by, default" )
            << select( Person.id ).from( Person ).orderBy( Person.PersonSurname, Person.PersonForename ).queryBuilder()
            << "SELECT tblPerson.id FROM tblPerson ORDER BY tblPerson.PersonSurname ASC, tblPerson.PersonForename ASC"
            << QVector<QVariant>();

        QTest::newRow( "double order by, mixed" )
            << select( Person.id ).from( Person ).orderBy( Person.PersonSurname, Qt::AscendingOrder, Person.PersonForename, Qt::DescendingOrder ).queryBuilder()
            << "SELECT tblPerson.id FROM tblPerson ORDER BY tblPerson.PersonSurname ASC, tblPerson.PersonForename DESC"
            << QVector<QVariant>();

        QTest::newRow( "placeholder condition" )
            << select( Person.id ).from( Person ).where( Person.id == placeholder(":foo") ).queryBuilder()
            << "SELECT tblPerson.id FROM tblPerson WHERE tblPerson.id = :foo"
            << QVector<QVariant>();

#ifdef _BullseyeCoverage
#pragma BullseyeCoverage off
#endif
        QTest::newRow( "placeholder + value condition" )
            << select( Person.id ).from( Person ).where( Person.id == placeholder(":foo") || Person.PersonSurname == QString::fromLatin1("bar") ).queryBuilder()
            << "SELECT tblPerson.id FROM tblPerson WHERE (tblPerson.id = :foo OR tblPerson.PersonSurname = :0)"
            << (QVector<QVariant>() << QLatin1String( "bar" ));
#ifdef _BullseyeCoverage
#pragma BullseyeCoverage on
#endif

         QTest::newRow( "IS NULL condition" )
            << select( Person.id ).from( Person ).where( isNull( Person.PersonForename ) ).queryBuilder()
            << "SELECT tblPerson.id FROM tblPerson WHERE tblPerson.PersonForename IS NULL"
            << QVector<QVariant>();

        QTest::newRow( "LIKE condition" )
            << select( Person.id ).from( Person ).where( like( Person.PersonForename, QLatin1String( "foo%" ) ) ).queryBuilder()
            << "SELECT tblPerson.id FROM tblPerson WHERE tblPerson.PersonForename LIKE :0"
            << (QVector<QVariant>() << QLatin1String( "foo%" ));

        QTest::newRow( "1 GROUP BY" )
            << select( Person.id ).from( Person ).groupBy( Person.PersonSurname, Person.id ).queryBuilder()
            << "SELECT tblPerson.id FROM tblPerson GROUP BY tblPerson.PersonSurname, tblPerson.id"
            << QVector<QVariant>();

        QTest::newRow( "2 GROUP BY" )
            << select( Person.id ).from( Person ).groupBy( Person.PersonSurname, Person.PersonForename, Person.id ).queryBuilder()
            << "SELECT tblPerson.id FROM tblPerson GROUP BY tblPerson.PersonSurname, tblPerson.PersonForename, tblPerson.id"
            << QVector<QVariant>();
    }

   void testSelect()
    {
        QFETCH( SqlSelectQueryBuilder, qb );
        QFETCH( QString, sql );
        QFETCH( QVector<QVariant>, bindVals );

        qb.query(); // trigger query assembly
        QCOMPARE( qb.m_queryString, sql );
        QCOMPARE( qb.m_bindValues, bindVals );
    }
};

QTEST_MAIN( SelectTest )

#include "selecttest.moc"
