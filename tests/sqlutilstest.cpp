#include "SqlUtils.h"
#include "test_utils.h"

#include <QObject>
#include <QtTest/QtTest>
#include <QStringList>

class SqlUtilsTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testStripComments_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("output");

        QTest::newRow("empty") << QString() << QString();
        QTest::newRow("comment1") << "-- hi there" << QString();
        QTest::newRow("comment2") << "-- hi there\n" << QString();
        QTest::newRow("comment3") << "-- hi there\n\n" << QString();
        QTest::newRow("query1") << "SELECT * FROM tblVersion;" << "SELECT * FROM tblVersion;";
        QTest::newRow("query2") << "SELECT * FROM tblVersion" << "SELECT * FROM tblVersion";
        QTest::newRow("query3") << "SELECT * FROM tblVersion\n" << "SELECT * FROM tblVersion";
        QTest::newRow("multi1") << "-- comment\nSELECT * FROM tblVersion" << "SELECT * FROM tblVersion";
        QTest::newRow("multi2") << "-- comment\nSELECT * FROM tblVersion;" << "SELECT * FROM tblVersion;";
        QTest::newRow("multi3") << "-- comment\nSELECT * FROM tblVersion\n" << "SELECT * FROM tblVersion";
    }

    void testStripComments()
    {
        QFETCH(QString, input);
        QFETCH(QString, output);

        SqlUtils::stripComments(input);
        QCOMPARE(input, output);
    }

    void testSplitQueries_data()
    {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QStringList>("output");

        QTest::newRow("empty") << QString() << QStringList();
        QTest::newRow("comment1") << "-- hi there" << (QStringList() << QL1S("-- hi there"));
        QTest::newRow("comment2") << "-- hi there\n" << (QStringList() << QL1S("-- hi there\n"));
        QTest::newRow("comment3") << "-- hi there\n\n" << (QStringList() << QL1S("-- hi there\n\n"));
        QTest::newRow("query1") << "SELECT * FROM tblVersion;" << (QStringList() << QL1S("SELECT * FROM tblVersion;"));
        QTest::newRow("query2") << "SELECT * FROM tblVersion;\n" << (QStringList() << QL1S("SELECT * FROM tblVersion"));
        QTest::newRow("query3") << "SELECT * FROM tblVersion\n" << (QStringList() << QL1S("SELECT * FROM tblVersion\n"));
        QTest::newRow("multi1") << "SELECT a FROM b;\nSELECT c FROM d" << (QStringList() << QL1S("SELECT a FROM b") << QL1S("SELECT c FROM d"));
    }

    void testSplitQueries()
    {
        QFETCH(QString, input);
        QFETCH(QStringList, output);

        const QStringList result = SqlUtils::splitQueries(input);
        QCOMPARE(result, output);
    }
};

QTEST_MAIN( SqlUtilsTest )

#include "sqlutilstest.moc"
