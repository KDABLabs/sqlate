#define QUERYBUILDER_UNITTEST

#include "testbase.h"
#include "testschema.h"
#include "Sql.h"
#include "SqlDeleteQueryBuilder.h"

#include <QObject>
#include <QtTest/QtTest>

Q_DECLARE_METATYPE( SqlDeleteQueryBuilder )
Q_DECLARE_METATYPE( QVector<QVariant> )

class DeleteQueryBuilderTest : public TestBase
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() {
      openDbTest();
      createEmptyDb();
    }

    void testQueryBuilder_data()
    {
        QTest::addColumn<SqlDeleteQueryBuilder>( "qb" );
        QTest::addColumn<QString>( "sql" );
        QTest::addColumn<QVector<QVariant> >( "values" );

        SqlDeleteQueryBuilder qb;
        QVector<QVariant> values;

        qb.setTable( Sql::Report );
        QTest::newRow( "Delete table" ) << qb << "DELETE FROM tblReport" << values;

        qb = SqlDeleteQueryBuilder();
        qb.setTable( Sql::Person );
        qb.setIncludeSubTables(false);
        QTest::newRow( "Delete only table" ) << qb << "DELETE FROM ONLY tblPerson" << values;

        qb = SqlDeleteQueryBuilder();
        qb.setTable( Sql::Workplace );
        qb.setIncludeSubTables(false);
        qb.whereCondition().addValueCondition( Sql::Workplace.itemorder, SqlCondition::Equals, 42 );
        values.clear();
        values << 42;
        QTest::newRow( "Delete, where clause, value" ) << qb << "DELETE FROM ONLY tblWorkplace WHERE tblWorkplace.itemorder = :0" << values;

    }

    void testQueryBuilder()
    {
        QFETCH( SqlDeleteQueryBuilder, qb );
        QFETCH( QString, sql );
        QFETCH( QVector<QVariant>, values );

        qb.query(); // trigger query assembly
        QCOMPARE( qb.m_queryString, sql );
        QCOMPARE( qb.m_bindValues, values);
    }
};

QTEST_MAIN( DeleteQueryBuilderTest )

#include "deletequerybuildertest.moc"
