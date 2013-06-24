#ifndef TESTBASE_H
#define TESTBASE_H

#include <QObject>

class QFileInfo;
/** Base class for unit tests, mostly for logging and resetting the postgres database
 */
class TestBase : public QObject
{
    Q_OBJECT    

protected slots:
    virtual void message(const QString& str);
    void cleanupTestCase();

protected:
    void openDbTest();
    void createEmptyDb();

    int getDbPort();
    QString databaseName() const;
    bool dropTables();
    bool executePreCreateScripts(const QString &path);
    bool processSqlFile(const QFileInfo &currentFileInfo);


};


#endif
