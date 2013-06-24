#include "testschema.h"
#include "Sql.h"
#include "SqlCreateRule.h"
#include "SqlGlobal.h"

#include <QObject>
#include <QtTest/QtTest>

class CreateRuleTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:

    void testRuleCreation()
    {
        const QStringList rules = Sql::createNotificationRuleStatements<Sql::SQLateTestSchema>();
        SQLDEBUG << rules;
        //NOTE: the test always takes only the first table, so adapt it if the table order changes in the schema
        QCOMPARE( rules[0], QLatin1String("CREATE OR REPLACE RULE tblVersionNotificationInsertRule AS ON INSERT TO tblVersion DO ALSO NOTIFY tblVersionChanged") );
        QCOMPARE( rules[1], QLatin1String("CREATE OR REPLACE RULE tblVersionNotificationUpdateRule AS ON UPDATE TO tblVersion DO ALSO NOTIFY tblVersionChanged") );
        QCOMPARE( rules[2], QLatin1String("CREATE OR REPLACE RULE tblVersionNotificationDeleteRule AS ON DELETE TO tblVersion DO ALSO NOTIFY tblVersionChanged") );

        //_1b57b54 is the encoded version of the "tblWorkplace.id" column
       // SQLDEBUG << rules;
        QRegExp rx(QLatin1String("^CREATE OR REPLACE RULE \"[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}\" "
                                 "AS ON UPDATE TO tblWorkplace DO ALSO SELECT pg_notify\\(CAST \\(OLD\\.id AS text\\) \\|\\| '_1b57b54',''\\)$"), Qt::CaseSensitive, QRegExp::RegExp2);

        SQLDEBUG << rules.indexOf(rx);
        QVERIFY(rules.indexOf(rx) != -1);

    }
};

QTEST_MAIN( CreateRuleTest )

#include "createruletest.moc"
