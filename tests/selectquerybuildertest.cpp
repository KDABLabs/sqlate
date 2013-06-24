#define QUERYBUILDER_UNITTEST

#include "testschema.h"
#include "Sql.h"
#include "SqlSelectQueryBuilder.h"

#include <QObject>
#include <QtTest/QtTest>
#include <QSqlError>

#include "test_utils.h"
#include "testbase.h"

#define QL1S(x) QString::fromLatin1(x)

Q_DECLARE_METATYPE( SqlSelectQueryBuilder )
Q_DECLARE_METATYPE( QVector<QVariant> )

using namespace Sql;

class SelectQueryBuilderTest : public TestBase
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase() {
      openDbTest();
      createEmptyDb();
      QSqlQuery query;
      QVERIFY(query.exec(QLatin1String("CREATE TABLE table1 (col1 VARCHAR(128), col2 VARCHAR(128), col3 VARCHAR(128), col4 VARCHAR(128));")));
      QVERIFY(query.exec(QLatin1String("CREATE TABLE table2 (col1_2 VARCHAR(128), col2_2 VARCHAR(128), col3_2 VARCHAR(128), col4_2 VARCHAR(128))")));
      QVERIFY(query.exec(QLatin1String("CREATE TABLE table3 (col1_3 VARCHAR(128), col2_3 VARCHAR(128), col3_3 VARCHAR(128), col4_3 VARCHAR(128))")));
    }

    void testQueryBuilder_data()
    {
        QTest::addColumn<SqlSelectQueryBuilder>( "qb" );
        QTest::addColumn<QString>( "sql" );
        QTest::addColumn<QVector<QVariant> >( "bindVals" );
        SqlSelectQueryBuilder qb;

        qb.setTable( QL1S("table1") );
        qb.addColumn( QL1S("col1") );
        QTest::newRow( "1 column" ) << qb << "SELECT col1 FROM table1" << QVector<QVariant>();

        qb.addColumn( QL1S( "col2" ) );
        QTest::newRow( "2 columns" ) << qb << "SELECT col1, col2 FROM table1" << QVector<QVariant>();

        qb.addColumn( QL1S( "col3" ), QL1S( "foo" ) );
        QTest::newRow( "3 columns with 1 alias" ) << qb << "SELECT col1, col2, col3 AS \"foo\" FROM table1" << QVector<QVariant>();

        qb = SqlSelectQueryBuilder();
        qb.addAllColumns();
        qb.setTable( QL1S( "table1" ) );
        qb.addSortColumn( QL1S("col1") );
        QTest::newRow( "sort 1 col" ) << qb << "SELECT * FROM table1 ORDER BY col1 ASC" << QVector<QVariant>();

        qb.addSortColumn( QL1S("col2"), Qt::DescendingOrder );
        QTest::newRow( "sort 2 cols" ) << qb << "SELECT * FROM table1 ORDER BY col1 ASC, col2 DESC" << QVector<QVariant>();

        qb = SqlSelectQueryBuilder();
        qb.addAllColumns();
        qb.setTable( Workplace );
        qb.whereCondition().addValueCondition( Workplace.itemorder, SqlCondition::Equals, 42 );
        QTest::newRow( "single where" ) << qb << "SELECT * FROM tblWorkplace WHERE tblWorkplace.itemorder = :0" << (QVector<QVariant>() << 42);

        qb.whereCondition().addColumnCondition( Workplace.contact_tel, SqlCondition::Greater, Workplace.contact_fax );
        QTest::newRow( "two and where conds" ) << qb << "SELECT * FROM tblWorkplace WHERE (tblWorkplace.itemorder = :0 AND tblWorkplace.contact_tel > tblWorkplace.contact_fax)" << (QVector<QVariant>() << 42);

        qb.whereCondition().setLogicOperator( SqlCondition::Or );
        QTest::newRow( "two or where conds" ) << qb << "SELECT * FROM tblWorkplace WHERE (tblWorkplace.itemorder = :0 OR tblWorkplace.contact_tel > tblWorkplace.contact_fax)" << (QVector<QVariant>() << 42);

        SqlCondition cond( SqlCondition::And );
        cond.addValueCondition( Workplace.short_desc, SqlCondition::Is, SqlNull );
        cond.addValueCondition( Workplace.description, SqlCondition::LessOrEqual, QString::fromLatin1("foo") );
        qb.whereCondition().addCondition( cond );
        QTest::newRow( "nested conds" ) << qb << "SELECT * FROM tblWorkplace WHERE (tblWorkplace.itemorder = :0 OR tblWorkplace.contact_tel > tblWorkplace.contact_fax OR (tblWorkplace.short_desc IS NULL AND tblWorkplace.description <= :1))" << (QVector<QVariant>() << 42 << QString::fromLatin1( "foo" ));

        qb = SqlSelectQueryBuilder();
        qb.addAllColumns();
        qb.setTable( QL1S( "table1" ) );
        qb.addJoin( SqlSelectQueryBuilder::InnerJoin, QL1S( "table2" ), QL1S( "col1" ), QL1S( "col2_2" ) );
        QTest::newRow( "1 inner join" ) << qb << "SELECT * FROM table1 INNER JOIN table2 ON col1 = col2_2" << QVector<QVariant>();

        qb.addJoin( SqlSelectQueryBuilder::InnerJoin, QL1S( "table3" ), QL1S( "col3" ), QL1S( "col4_3" ) );
        QTest::newRow( "2 inner joins" ) << qb << "SELECT * FROM table1 INNER JOIN table2 ON col1 = col2_2 INNER JOIN table3 ON col3 = col4_3" << QVector<QVariant>();

        qb = SqlSelectQueryBuilder();
        qb.setTable( QL1S( "table1" ) );
        qb.addColumn( QL1S( "col1" ));
        qb.addGroupColumn( QL1S( "table1.col1" ) );
        QTest::newRow( "1 group by" ) << qb << "SELECT col1 FROM table1 GROUP BY table1.col1" << QVector<QVariant>();

        qb.addColumn( QL1S( "col2" ));
        qb.addGroupColumn( QL1S( "table1.col2" ) );
        QTest::newRow( "2 group by" ) << qb << "SELECT col1, col2 FROM table1 GROUP BY table1.col1, table1.col2" << QVector<QVariant>();

        qb = SqlSelectQueryBuilder();
        qb.addAllColumns();
        qb.setTable( QL1S( "table1" ) );
        qb.whereCondition().addPlaceholderCondition( QL1S( "col1" ), SqlCondition::Equals, QL1S( ":p" ) );
        QTest::newRow( "placeholder" ) << qb << "SELECT * FROM table1 WHERE col1 = :p" << QVector<QVariant>();

        qb = SqlSelectQueryBuilder();
        qb.addAllColumns();
        qb.setTable( Report );
        qb.whereCondition().addValueCondition( Report.txt, SqlCondition::Like, QL1S( "%foo" ) );
        QTest::newRow( "LIKE condition" ) << qb << "SELECT * FROM tblReport WHERE tblReport.txt LIKE :0" << (QVector<QVariant>() << QL1S( "%foo" ));

        qb = SqlSelectQueryBuilder();
        qb.addAllColumns();
        qb.setTable( Workplace );
        qb.lockExclusive( Workplace );
        QTest::newRow( "LIKE condition" ) << qb << "SELECT * FROM tblWorkplace FOR UPDATE OF tblWorkplace" << QVector<QVariant>();
        
        qb = SqlSelectQueryBuilder();
        qb.addAllColumns();
        qb.setTable( Workplace );
        qb.tryLockExclusive( Workplace );
        QTest::newRow( "LIKE condition" ) << qb << "SELECT * FROM tblWorkplace FOR UPDATE OF tblWorkplace NOWAIT" << QVector<QVariant>();

        qb = SqlSelectQueryBuilder();
        qb.setDistinct(true);
        qb.addColumn(QL1S("col1"));
        qb.setTable(QL1S( "table1" ));
        QTest::newRow( "DISTINCT" ) << qb << "SELECT DISTINCT col1 FROM table1" << QVector<QVariant>();

        qb = SqlSelectQueryBuilder();
        qb.setDistinctOn(QL1S("col2"));
        qb.addColumn(QL1S("col1"));
        qb.setTable(QL1S( "table1" ));
        QTest::newRow( "DISTINCT ON" ) << qb << "SELECT DISTINCT ON(col2) col1 FROM table1" << QVector<QVariant>();


        qb = SqlSelectQueryBuilder();
        qb.setTable( QL1S("table1") );
        qb.addColumn( QL1S("col1") );

        SqlSelectQueryBuilder qb2;
        qb2.setTable( QL1S("table2") );
        qb2.addColumn( QL1S("col2_2") );

        SqlSelectQueryBuilder qbCombined;
        qbCombined.combineQueries(qb, qb2);
        QTest::newRow( "UNION " ) << qbCombined << "SELECT col1 FROM table1 UNION SELECT col2_2 FROM table2" << QVector<QVariant>();

        SqlSelectQueryBuilder qbCombinedAll;
        qbCombinedAll.combineQueries(qb, qb2, SqlSelectQueryBuilder::UnionAll);
        QTest::newRow( "UNION ALL" ) << qbCombinedAll << "SELECT col1 FROM table1 UNION ALL SELECT col2_2 FROM table2" << QVector<QVariant>();

        qb = SqlSelectQueryBuilder();
        qb.setTable(Report);
        qb.addAllColumns();
        qb.whereCondition().addValueCondition(Report.ts, SqlCondition::LessOrEqual, SqlNow);
        qb.whereCondition().addValueCondition(Report.txt, SqlCondition::Is, SqlNull);
        QTest::newRow( "server side time" ) << qb << "SELECT * FROM tblReport WHERE (tblReport.ts <= now() AND tblReport.txt IS NULL)" << QVector<QVariant>();
    }

    void testQueryBuilder()
    {
        QFETCH( SqlSelectQueryBuilder, qb );
        QFETCH( QString, sql );
        QFETCH( QVector<QVariant>, bindVals );

        qb.query(); // trigger query assembly
        QCOMPARE( qb.m_queryString, sql );
        // variant comparission for custom types (here, mostly QUuid) doesn't work, so we have to do this manually...
        QCOMPARE( qb.m_bindValues.size(), bindVals.size() );
        QVERIFY( std::equal( qb.m_bindValues.begin(), qb.m_bindValues.end(), bindVals.begin(), deepVariantCompare ) );
    }
};

QTEST_MAIN( SelectQueryBuilderTest )

#include "selectquerybuildertest.moc"
