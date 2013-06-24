#define QUERYBUILDER_UNITTEST

#include "testschema.h"
#include "testbase.h"
#include "Sql.h"
#include "SqlUpdateQueryBuilder.h"
#include "test_utils.h"

#include <QObject>
#include <QtTest/QtTest>

Q_DECLARE_METATYPE( SqlUpdateQueryBuilder )
Q_DECLARE_METATYPE( QVector<QVariant> )

using namespace Sql;

class UpdateQueryBuilderTest : public TestBase
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() {
      openDbTest();
      createEmptyDb();
      QSqlQuery query;
      QVERIFY(query.exec(QLatin1String("CREATE TABLE table1 (first VARCHAR(128), second INTEGER)")));
    }
  
    void testQueryBuilder_data()
    {
        QTest::addColumn<SqlUpdateQueryBuilder>( "qb" );
        QTest::addColumn<QString>( "sql" );
        QTest::addColumn<QStringList >( "columns" );
        QTest::addColumn<QVector<QVariant> >( "values" );

        SqlUpdateQueryBuilder qb;
        QStringList columns;
        QVector<QVariant> values;

        qb.setTable( QL1S("table1") );
        qb.addColumnValue( QL1S("first"), QL1S("1") );
        columns << QL1S("first");
        values << QL1S("1");
        QTest::newRow( "Single column" ) << qb << "UPDATE table1 SET first = :0" << columns << values;

        qb = SqlUpdateQueryBuilder();
        qb.setTable( QL1S("table1") );
        qb.addColumnValue( QL1S("first"), QL1S("foo") );
        qb.addColumnValue( QL1S("second"), 42 );
        columns.clear();
        values.clear();
        columns << QL1S("first") << QL1S("second");        
        values << QL1S("foo") << 42;
        QTest::newRow( "2 columns, string and number" ) << qb << "UPDATE table1 SET first = :0, second = :1" << columns << values;

        qb = SqlUpdateQueryBuilder();
        qb.setTable( QL1S("table1") );
        qb.setIncludesubTales(false);
        qb.addColumnValue( QL1S("first"), QL1S("foo") );
        qb.addColumnValue( QL1S("second"), 42 );
        columns.clear();
        values.clear();
        columns << QL1S("first") << QL1S("second");        
        values << QL1S("foo") << 42;
        QTest::newRow( "2 columns, no subtable update" ) << qb << "UPDATE ONLY table1 SET first = :0, second = :1" << columns << values;

        qb = SqlUpdateQueryBuilder();
        qb.setTable( QL1S("table1") );
        qb.setIncludesubTales(false);
        qb.addColumnValue( QL1S("first"), QL1S("foo") );
        qb.addColumnValue( QL1S("second"), 42 );
        qb.whereCondition().addPlaceholderCondition( QL1S( "first" ), SqlCondition::Equals, QL1S( ":p" ) );
        columns.clear();
        values.clear();
        columns << QL1S("first") << QL1S("second");        
        values << QL1S("foo") << 42;
        QTest::newRow( "Where clause, placeholder" ) << qb << "UPDATE ONLY table1 SET first = :0, second = :1 WHERE first = :p" << columns << values;

        qb = SqlUpdateQueryBuilder();
        qb.setTable( Workplace );
        qb.setIncludesubTales(false);
        qb.addColumnValue( Workplace.description, QL1S("foo") );
        qb.addColumnValue( Workplace.itemorder, 42 );
        qb.whereCondition().addValueCondition( Workplace.short_desc, SqlCondition::Equals, QL1S( "bar" ) );
        columns.clear();
        values.clear();
        columns << QL1S("description") << QL1S("itemorder");
        values << QL1S("foo") << 42 << QL1S( "bar" );
        QTest::newRow( "Where clause, value" ) << qb << "UPDATE ONLY tblWorkplace SET description = :0, itemorder = :1 WHERE tblWorkplace.short_desc = :2" << columns << values;

        qb = SqlUpdateQueryBuilder();
        qb.setTable( Report );
        qb.addColumnValue( Report.ts, SqlNow );
        qb.addColumnValue( Report.txt, SqlNull );
        columns.clear();
        values.clear();
        columns << QL1S("ts") << QL1S("txt");
        values << QVariant();
        QTest::newRow( "server-side now" ) << qb << "UPDATE tblReport SET ts = now(), txt = :0" << columns << values;
    }

    void testQueryBuilder()
    {
        QFETCH( SqlUpdateQueryBuilder, qb );
        QFETCH( QString, sql );
        QFETCH( QStringList, columns );
        QFETCH( QVector<QVariant>, values );

        qb.query(); // trigger query assembly
        QCOMPARE( qb.m_queryString, sql );
        QCOMPARE( qb.columnNames(), columns );
        // variant comparission for custom types (here, mostly QUuid) doesn't work, so we have to do this manually...
        QCOMPARE( qb.m_bindValues.size(), values.size() );
        QVERIFY( std::equal( qb.m_bindValues.begin(), qb.m_bindValues.end(), values.begin(), deepVariantCompare ) );
    }
};

QTEST_MAIN( UpdateQueryBuilderTest )

#include "updatequerybuildertest.moc"
