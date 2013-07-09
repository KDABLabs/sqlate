#include <QtTest/QtTest> //Windows needs it as the first include

#include "SchemaUpdater.h" //must be the first include to avoid ambigous operator overload warnings

#include "testschema.h"
#include "testbase.h"
#include "OldSchema.h"
namespace OldSql {
    DEFINE_SCHEMA( OLDSCHEMA )
}
#include "NewSchema.h"
namespace NewSql {
    DEFINE_SCHEMA( NEWSCHEMA )
}

#include "SqlSchema.h"
#include "SqlQuery.h"
#include "SqlSelect.h"
#include "SqlInsertQueryBuilder.h"
#include "SqlUtils.h"
#include "SqlExceptions.h"

#include <QObject>
#include <QSqlDatabase>
#include <QSqlDriver>
#include <QString>

using namespace Sql;

class SchemaUpdateTest : public TestBase
{
    Q_OBJECT

    private:
        int currentVersion()
        {
            SqlQuery q = select( Version.version ).from( Version );
            q.exec();
            q.next();
            return q.value(0).toInt();
        }

        bool containsColumn( const QVector<QPair<QString, QVariant::Type> > & cols, const char* name, QVariant::Type type )
        {
            typedef QPair<QString, QVariant::Type> StringTypePair;
            foreach ( const StringTypePair &c, cols ) {
                if ( c.first == QLatin1String(name) && c.second == type )
                    return true;
            }
            return false;
        }

    private slots:
        void initTestCase()
        {
              openDbTest();

              try {
                  SqlQuery q;
                  q.exec(QLatin1String("SET client_min_messages TO WARNING;"));
              } catch ( const SqlException &e ) {
                  qDebug() << tr("Log messages cannot be turned off: %1").arg( e.error().text() );
              }

              dropTables();

              executePreCreateScripts(QLatin1String(":/sql"));
              
              try {
              foreach (const QString &stmt, Sql::createTableStatements<OldSql::OldSchema>()) {
                  SqlQuery q;
                  q.exec( stmt );
              }
              } catch ( const SqlException& e ) {
                  qDebug() << "SQL Error on table creation: " << e.error().text();
                  exit(1);
              }

              SqlInsertQueryBuilder qb;
              qb.setTable( OldSql::Version );
              qb.addColumnValue( OldSql::Version.version, 1 );
              qb.exec();

              try {
                  SqlQuery q;
                  q.exec(QLatin1String("SET client_min_messages TO NOTICE;"));
              } catch ( const SqlException &e ) {
                  qDebug() << tr("Log messages cannot be turned off: %1").arg( e.error().text() );
              }

        }

        void testSchemaUpdate()
        {
            QCOMPARE( currentVersion(), 1 );

            const QSqlDatabase db = QSqlDatabase::database();

            // already correct version
            SchemaUpdater<OldSql::OldSchema, VersionType> updater1(1);
            QVERIFY(!updater1.needsUpdate());
            updater1.execUpdates(false);
            QCOMPARE( currentVersion(), 1 );
            QCOMPARE( db.tables().size(), 2 );

            SchemaUpdater<NewSql::NewSchema, VersionType> updater2(3);
            QVERIFY(updater2.needsUpdate());
            updater2.execUpdates(false);
            QCOMPARE( currentVersion(), 3 );
            QCOMPARE( db.tables().size(), 3 );

            QVector<QPair<QString, QVariant::Type> > cols = SqlUtils::columnsOfTable( OldSql::Existing, db );
            QCOMPARE( cols.size(), 3 );
            QVERIFY( containsColumn( cols, "id", QVariant::String ) );
            QVERIFY( containsColumn( cols, "column1", QVariant::String ) );
            QVERIFY( containsColumn( cols, "column2", QVariant::Int ) );

            cols = SqlUtils::columnsOfTable( NewSql::New );
            QCOMPARE( cols.size(), 2 );
            QVERIFY( containsColumn( cols, "column1", QVariant::String ) );
        }

};

QTEST_MAIN( SchemaUpdateTest )

#include "schemaupdatetest.moc"
