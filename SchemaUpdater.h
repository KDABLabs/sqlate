/*
    Copyright (C) 2011-2013 Klarälvdalens Datakonsult AB,
        a KDAB Group company, info@kdab.net,
        author Volker Krause <volker.krause@kdab.com>

    This library is free software; you can redistribute it and/or modify it
    under the terms of the GNU Library General Public License as published by
    the Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    This library is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
    License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to the
    Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301, USA.
*/
#ifndef SCHEMAUPDATER_H
#define SCHEMAUPDATER_H
#include "sqlate_export.h"

#include "SqlCreateTable.h"
#include "SqlUtils.h"
#include "SqlExceptions.h"
#include "SqlQueryCache.h"
#include "SqlUpdateQueryBuilder.h"
#include "SqlTransaction.h"

#include <QMessageBox>
#include <QApplication>
#include <QVector>
#include <QString>
#include <QObject>
#include <QDir>


/**
 * @internal
 * Methods for performing the database schema upgrade.
 * Connect the signals defined in here to the log window.
 *
 * @note Use SchemaUpdater instead!
 */

class SQLATE_EXPORT SchemaUpdaterBase : public QObject
{
    Q_OBJECT
public:
    /**
     * @param targetVersion The version we want to upgrade to.
     * @param parentWidget widget used as dialog parent
     */
    explicit SchemaUpdaterBase( int targetVersion, const QString & pluginName = QString(), QWidget* parentWidget = 0, QObject *parent = 0 );

    /**
     * Returns @c true if the schema needs to be updated.
     * @param the table in which the version of the schema is stored
     * @note 1 table for the main app, 1 table for each plugin
     */
    virtual bool needsUpdate() const = 0;

    /**
     * Executes all necessary database schema updates.
     * @param interactive Ask user for confirmation.
     */
    virtual void execUpdates(bool interactive) = 0;


signals:
    void infoMessage(const QString &msg) const;
    void successMessage(const QString &msg) const;
    void errorMessage(const QString &msg) const;

protected:
    virtual void createMissingTables() = 0;
    virtual void createMissingColumns() = 0;
    virtual void createRulesPermissionsAndTriggers() = 0;

    struct UpdateInfo {
        int version;
        QString updateFile;
    };

    /**
     * Lists all necessary updates to execute
     * @param dir: the directory containing the scripts files
     */
    QVector<UpdateInfo> pendingUpdates(const QDir & dir) const;

    /**
     * Executes a single update step.
     * @throws SqlException on query errors.
     */
    void execUpdate(const QString &updateFile);

    QWidget *m_parentWidget;
    mutable int m_currentVersion;
    const int m_targetVersion;
    QString m_pluginName;
};


namespace detail {

/**
 * Helper class for MPL iteration to create missing columns.
 * @internal
 */
struct missing_column_creator
{
    explicit missing_column_creator( const QSqlDatabase &db ) : m_db( db ) {}

    /**
     * Determines the missing columns and creates them.
     * @tparam Table The table type.
     */
    template <typename Table>
    void operator() ( const Table &table )
    {
        typedef QPair<QString, QVariant::Type> StringTypePair;
        const QVector<StringTypePair> existingColumns = SqlUtils::columnsOfTable(table, m_db);

        QStringList columnNames;
        Sql::detail::sql_name_accumulator nameAccu( columnNames );
        boost::mpl::for_each<typename Table::columns, Sql::detail::wrap<boost::mpl::placeholders::_1> >( nameAccu );

        QStringList columnStatements;
        Sql::detail::column_creator stmtAccu( columnStatements );
        boost::mpl::for_each<typename Table::columns, Sql::detail::wrap<boost::mpl::placeholders::_1> >( stmtAccu );

        Q_ASSERT(columnNames.size() == columnStatements.size());
        for ( QStringList::const_iterator it = columnNames.constBegin(), it2 = columnStatements.constBegin(); it != columnNames.constEnd(); ++it, ++it2 ) {
            bool found = false;
            foreach ( const StringTypePair &existinCol, existingColumns ) {
                if ( existinCol.first.toLower() == (*it).toLower() ) {
                    found = true;
                    break;
                }
            }
            if (found)
                continue;

            QString alterStmt = QLatin1Literal("ALTER TABLE ") % Table::tableName() % QLatin1Literal(" ADD COLUMN ") % *it2;
            SqlQuery query(m_db);
            query.exec(alterStmt);
        }
    }

    QSqlDatabase m_db;
};


} // detail


/**
 * Database schema updater.
 * Additionally to SchemaUpdaterBase this also adds missing tables and columns based on the given schema.
 */
template <typename Schema, typename tableVersion>
class SchemaUpdater : public SchemaUpdaterBase
{
public:
    /**
     * @param targetVersion The version we want to upgrade to.
     * @param parentWidget widget used as dialog parent
     */
    explicit SchemaUpdater( int targetVersion, const QString & pluginName = QString(), QWidget* parentWidget = 0, QObject *parent = 0 ) :
        SchemaUpdaterBase(targetVersion,pluginName, parentWidget, parent)
        {}

    /**
     * Returns @c true if the schema needs to be updated.
     * @param the table in which the version of the schema is stored
     * @note 1 table for the main app, 1 table for each plugin
     */
    bool needsUpdate() const
   {
       tableVersion table;
       if (m_currentVersion >= 0)
           return m_currentVersion < m_targetVersion;

       try {
           SqlQuery q = select( table.version ).from( table );
           q.exec();
           if (q.next()) {
               m_currentVersion = q.value(0).toInt();
               Q_ASSERT(m_currentVersion >= 0);
               return m_currentVersion < m_targetVersion;
           }
       } catch (const SqlException& e) {
            emit errorMessage( QObject::tr("<html>There was an error while checking the database version. This is a critical error, the application will terminate.<br/>\
                                 The error was: <b>%1</b>.</html>").arg(e.error().text()) );
             QApplication::exit(1);
             return false;
       }

       Q_ASSERT(false); // this case is caught way earlier
       return true;
   }

    /**
     * Executes all necessary database schema updates.
     * @param interactive Ask user for confirmation.
     */
    void execUpdates(bool interactive)
    {
        SqlQueryCache::setEnabled(false);

        tableVersion table;
        const QString schemaName = m_pluginName.isNull()?tr("Main application"):m_pluginName;
        if (!needsUpdate()) {
            emit successMessage(QObject::tr("schema is up-to-date for %1.").arg(schemaName));
            return;
        }
        emit errorMessage(QObject::tr("schema is outdated for %1.").arg(schemaName));

        if (interactive && QMessageBox::warning( m_parentWidget, QObject::tr("Database Schema Update Required"),
                QObject::tr("An update to the database schema is required to proceed. Such an update can take considerable time and will require updates to all client installations as well. "
                            "Also, ensure you have current backups before proceeding."), QMessageBox::Apply | QMessageBox::Abort, QMessageBox::Abort)
            != QMessageBox::Apply)
        {
            QApplication::quit();
            return;
        }

        // step 1: auto-create missing tables
        try {
            createMissingTables();
        } catch (const SqlException &e) {
            emit errorMessage(QObject::tr("Database schema update failed during creation of new tables: %1.").arg(e.error().text()));
            QApplication::exit(1);
            return;
        }

        // step 2 auto-create missing columns, if the previous step succeeded this must not fail due to missing tables
        try {
            createMissingColumns();
        } catch (const SqlException &e) {
            emit errorMessage(QObject::tr("Database schema update failed during creation of new columns: %1.").arg(e.error().text()));
            QApplication::exit(1);
            return;
        }

        // step 3: run custom update scripts
        foreach (const UpdateInfo &update, pendingUpdates(QDir(QString::fromLatin1(":/%1schemaUpdates/").arg(m_pluginName)))) {
            try {
                emit infoMessage(QObject::tr("Upgrading schema to version %1...").arg(update.version));
                SqlTransaction t;

                execUpdate(update.updateFile);

                SqlUpdateQueryBuilder qb;
                qb.setTable(table);
                qb.addColumnValue(table.version, update.version);
                qb.exec();

                t.commit();
                m_currentVersion = update.version;
                emit successMessage(QObject::tr("Schema successfully upgraded to version %1").arg(update.version));
            } catch (const SqlException &e) {
                emit errorMessage(QObject::tr("Schema update failed to version %1. The error was '%2'.").arg(update.version).arg(e.error().text()));
                QApplication::exit(1);
                return;
            }
        }

        // step 3 create the rules/triggers for tables. Must be done at the end, after the tables are fully updated
        try {
            createRulesPermissionsAndTriggers();
        } catch (const SqlException &e) {
            emit errorMessage(QObject::tr("Database schema update failed during creation of new columns: %1").arg(e.error().text()));
            QApplication::exit(1);
            return;
        }
       
        // step 4: if necessary increase the version number beyond what the custom updates did, in case the latest version
        // only included automatic changes.
        if (m_currentVersion < m_targetVersion) {
            try {
                SqlUpdateQueryBuilder qb;
                qb.setTable(table);
                qb.addColumnValue(table.version, m_targetVersion);
                qb.exec();
                m_currentVersion = m_targetVersion;
            } catch (const SqlException &e) {
                emit errorMessage(QObject::tr("The database schema version number could not be updated: %1.").arg(e.error().text()));
                QApplication::exit(1);
                return;
            }
        }

        SqlQueryCache::setEnabled(true);
    }

protected:
    void createMissingTables()
    {
        Sql::createMissingTables<Schema>(QSqlDatabase::database());
    }

    void createMissingColumns()
    {
        ::detail::missing_column_creator creator(QSqlDatabase::database());
        boost::mpl::for_each<Schema>( creator );
    }
    
    void createRulesPermissionsAndTriggers()
    {
        Sql::createRulesPermissionsAndTriggers<Schema>(QSqlDatabase::database());
    }
};


#endif // SCHEMAUPDATER_H
