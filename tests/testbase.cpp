#include "testbase.h"

#include "testschema.h"
#include "SqlExceptions.h"
#include "SqlQueryCache.h"
#include "SqlGlobal.h"
#include "SqlCreateTable.h"
#include "Sql.h"

#include <QtTest/QTest>
#include <QFileInfo>
#include <QMetaObject>
#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDir>

QString TestBase::databaseName() const
{
    const QString dbName = QLatin1String( qgetenv("SQLATE_DATABASE_NAME").constData() );
    if (dbName.isEmpty()) {
        return QLatin1String("sqlate_test");
    }

    return dbName.toLower();
}

void TestBase::openDbTest()
{
    QString dbName = databaseName();
    QSqlDatabase db = QSqlDatabase::addDatabase(QLatin1String( "QPSQL" ));
    db.setDatabaseName( dbName );
     // Allow override by env settings
    // No reason to restrict as only setting where the app connects to
    QString dbHost = QLatin1String( qgetenv("SQLATE_DATABASE_HOST").constData() );
    if (dbHost.isEmpty()) {
        dbHost = QLatin1String("127.0.0.1");
    }

    QString dbUser = QLatin1String( "sqlate.user" );
    QString dbPassword = QLatin1String("sql123456" );
    int dbPort = getDbPort();
    qDebug() << "Using database: " << dbName << " on host " << dbHost << " port: " << dbPort;
    db.setHostName( dbHost );
    db.setPort( dbPort );
    db.setUserName( dbUser );
    db.setPassword( dbPassword );

    if ( !db.open() ) {
        //try to create the database
        db.close();
        QSqlDatabase db2 = QSqlDatabase::database(  );
        db2.setDatabaseName( QLatin1String("template1") );
        db2.setHostName(dbHost);
        db2.setPort(dbPort);
        db2.setUserName(dbUser);
        db2.setPassword(dbPassword);
        if ( !db2.open() ) {
            qFatal("Failed to connect: %s", db2.lastError().text().toLatin1().constData());
        }

        QString s = QString( QLatin1String("CREATE DATABASE %1 ENCODING 'UTF8' LC_COLLATE='en_US.UTF-8' LC_CTYPE='en_US.UTF-8' TEMPLATE template0") ).arg( databaseName() );
        QSqlQuery query( db2 );
        if ( !query.exec(s) ) {
            qFatal("Failed to create the %s database: %s", databaseName().toLatin1().constData(), query.lastError().text().toLatin1().constData());
        }
        db2.close();
        //reconnect again
        db.close();
        db = QSqlDatabase::database( );
        db.setDatabaseName(databaseName());
        db.setHostName(dbHost);
        db.setPort(dbPort);
        db.setUserName(dbUser);
        db.setPassword(dbPassword);

        if ( !db.open() ) {
            qFatal("Failed to connect: %s", db.lastError().text().toLatin1().constData());
        }
    }
    qDebug() << db;
    /* else {
        qFatal("Failed to connect: %s", db.lastError().text().toLatin1().constData());
    }*/
}

bool TestBase::executePreCreateScripts(const QString &path)
{
    const QString proceduresFilesPath = QString::fromLatin1("%1/procedures/pre-create").arg(path);

    QFileInfoList preCreateFiles;
    // Make list of sql files to process
    if ( path.length() ) {
        QDir preCreateFilesDir(proceduresFilesPath, QLatin1String( "*.tsp" ));
        preCreateFilesDir.setSorting( QDir::Name );
        preCreateFiles.append(preCreateFilesDir.entryInfoList(QDir::Files));
    } else {
        message( tr( "Cannot find static data and procedures in %1").arg(path));
        return false;
    }

    QFileInfoList::const_iterator sqlfileslist_it;
    for (sqlfileslist_it=preCreateFiles.constBegin(); sqlfileslist_it!=preCreateFiles.constEnd(); ++sqlfileslist_it) {
        bool result = processSqlFile( *sqlfileslist_it );
        if (!result) {
            return false;
        }
    }
    return true;
}

bool TestBase::processSqlFile(const QFileInfo &currentFileInfo)
{
    message(tr( " Processing file: %1").arg(currentFileInfo.absoluteFilePath()));
    QFile file( currentFileInfo.absoluteFilePath() );
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        message( tr( " could not open file: %1").arg(currentFileInfo.absoluteFilePath()));
        return false;
    }
    else {
        message( tr( "... OK"));
    }

    QTextStream in(&file);
    QString sql = in.readAll();

    SqlUtils::stripComments(sql);
    if ( sql.isEmpty() )
        return true;

    SqlQuery query;
    query.exec( sql );
    return true;
}


void TestBase::createEmptyDb()
{
    try {
        SqlQuery q;
        q.exec(QLatin1String("SET client_min_messages TO WARNING;"));
    } catch ( const SqlException &e ) {
        qDebug() << tr("Log messages cannot be turned off: %1").arg( e.error().text() );
    }

    dropTables();

    executePreCreateScripts(QLatin1String(":/sql"));
    // create tables
    try {
        Sql::createMissingTables<Sql::SQLateTestSchema>();
    } catch ( const SqlException &e ) {
        message( tr("Error while creating the tables: %1").arg(e.error().text()) );
        return;
    }

    try {
        Sql::createRulesPermissionsAndTriggers<Sql::SQLateTestSchema>();
    } catch ( const SqlException &e ) {
        message( tr("Error while creating the table rules, triggers and setting the permissions: %1").arg(e.error().text()) );
        return;
    }
}

bool TestBase::dropTables()
{
    message(tr("Dropping all tables..."));

    foreach ( const QString &table, QSqlDatabase::database().tables() ) {
        try {
            SqlQuery q;
            q.exec(QString::fromLatin1( "DROP TABLE IF EXISTS %1 CASCADE" ).arg( table ));
        } catch ( const SqlException &e ) {
           message( tr("Dropping table %1 failed: %2").arg( table ).arg( e.error().text() ) );
           return false;
        }
    }
   message(tr("All tables dropped."));

   return true;
}


void TestBase::message(const QString& str)
{
    SQLDEBUG << str;
}

void TestBase::cleanupTestCase()
{
    SqlQueryCache::clear();
}

int TestBase::getDbPort()
{
    int dbPort = QString::fromLatin1( qgetenv("SQL_DATABASE_HOST_PORT").constData() ).toInt();
    if (dbPort <= 0 ) {
        dbPort = SQLATE_DEFAULT_SERVER_PORT;
    }
    return dbPort;
}


#include "moc_testbase.cpp"
