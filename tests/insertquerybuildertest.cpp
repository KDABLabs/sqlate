#define QUERYBUILDER_UNITTEST

#include "test_utils.h"
#include "testbase.h"
#include "testschema.h"

#include "Sql.h"
#include "SqlInsertQueryBuilder.h"

#include <QObject>
#include <QtTest/QtTest>

#include <algorithm>

using namespace Sql;

Q_DECLARE_METATYPE( SqlInsertQueryBuilder )
Q_DECLARE_METATYPE( QVector<QVariant> )

class InsertQueryBuilderTest : public TestBase
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
        QTest::addColumn<SqlInsertQueryBuilder>( "qb" );
        QTest::addColumn<QString>( "sql" );
        QTest::addColumn<QStringList >( "columns" );
        QTest::addColumn<QVector<QVariant> >( "values" );

        SqlInsertQueryBuilder qb;
        QStringList columns;
        QVector<QVariant> values;

        qb.setTable( Report );
        qb.setToDefaultValues();
        QTest::newRow( "Default values" ) << qb << "INSERT INTO tblReport DEFAULT VALUES" << columns << values;

        qb = SqlInsertQueryBuilder();
        qb.setTable( QL1S("table1"));
        qb.addColumnValue( QL1S("first"), QL1S("1") );
        columns.clear();
        values.clear();
        columns << QL1S("first");    
        values << QL1S("1");
        QTest::newRow( "1 column, string" ) << qb << "INSERT INTO table1 (first) VALUES (:0)" << columns << values;

        qb = SqlInsertQueryBuilder();
        qb.addColumnValue( QL1S("first"), QL1S("1") );
        qb.addColumnValue( QL1S("second"), 42 );
        qb.setTable( QL1S("table1") );
        
        columns.clear();
        values.clear();
        columns << QL1S("first") << QL1S("second");        
        values << QL1S("1") << 42;
        QTest::newRow( "2 columns, string and number" ) << qb << "INSERT INTO table1 (first,second) VALUES (:0,:1)" << columns << values;

        qb = SqlInsertQueryBuilder();
        qb.addColumnValue( QL1S("first"), QL1S("1") );
        qb.addColumnValue( QL1S("second"), QVariant() );
        qb.setTable( QL1S("table1") );
        columns.clear();
        values.clear();
        columns << QL1S("first") << QL1S("second");
        values << QL1S("1") << QVariant();
        QTest::newRow( "2 columns, string and number, number is NULL" ) << qb << "INSERT INTO table1 (first,second) VALUES (:0,:1)" << columns << values;

        qb = SqlInsertQueryBuilder();
        qb.setTable( Person );
        qb.addColumnValue( Person.id, QUuid("a12352e5-28e1-4873-a2a0-6c83c3c4b75a") );
        columns.clear();
        values.clear();
        columns << QL1S("id");
        values << QVariant::fromValue( QUuid("a12352e5-28e1-4873-a2a0-6c83c3c4b75a") );
        QTest::newRow( "1 column, uuid, template" ) << qb << "INSERT INTO tblPerson (id) VALUES (:0)" << columns << values;

        qb = SqlInsertQueryBuilder();
        qb.setTable( Report );
        QUuid hoId = QUuid::createUuid();
        qb.addColumnValue(Report.id, hoId);
        qb.addColumnValue(Report.ts, SqlNow);
        qb.addColumnValue(Report.txt, SqlNull);
        columns.clear();
        values.clear();
        columns << QL1S("id") << QL1S("ts") << QL1S("txt");
        values << QVariant::fromValue( hoId ) << QVariant::fromValue<SqlNowType>(SqlNow) << QVariant();
        QTest::newRow("server-side now") << qb << "INSERT INTO tblReport (id,ts,txt) VALUES (:0,now(),:2)" << columns << values;
    }

    void testQueryBuilder()
    {
        QFETCH( SqlInsertQueryBuilder, qb );
        QFETCH( QString, sql );
        QFETCH( QStringList, columns );
        QFETCH( QVector<QVariant>, values );

        qb.query(); // trigger query assembly
        QCOMPARE( qb.m_queryString, sql );
        QCOMPARE( qb.m_columnNames, columns );
        qDebug() << qb.m_values << values;
        // variant comparission for custom types (here, mostly QUuid) doesn't work, so we have to do this manually...
        QCOMPARE( qb.m_values.size(), values.size() );
        QVERIFY( std::equal( qb.m_values.begin(), qb.m_values.end(), values.begin(), deepVariantCompare ) );
    }
};

QTEST_MAIN( InsertQueryBuilderTest )

#include "insertquerybuildertest.moc"
