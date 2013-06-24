#include "testschema.h"
#include "Sql.h"

#include <QObject>
#include <QtTest/QtTest>

#include <boost/mpl/assert.hpp>
#include <boost/mpl/not.hpp>

class SchemaTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testIdentifierAccess()
    {
        QCOMPARE( Sql::Person.sqlName(), QLatin1String( "tblPerson" ) );
        QCOMPARE( Sql::PersonType::sqlName(), QLatin1String( "tblPerson" ) );
        QCOMPARE( Sql::Person.tableName(), QLatin1String( "tblPerson" ) );

        QCOMPARE( Sql::Person.id.sqlName(), QLatin1String( "id" ) );
        QCOMPARE( Sql::Person.id.name(), QLatin1String( "tblPerson.id" ) );
        QCOMPARE( Sql::PersonType::idType::name(), QLatin1String( "tblPerson.id" ) );
    }

    void testTypeAccess()
    {
        BOOST_MPL_ASSERT(( boost::is_same<Sql::PersonType::idType::type, QUuid> ));
        BOOST_MPL_ASSERT(( boost::is_same<Sql::PersonType::PersonGradeType::referenced_column, Sql::PersonGradesType::idType> ));
    }

    void testColumnPropertyAccess()
    {
        BOOST_MPL_ASSERT(( boost::mpl::not_<Sql::PersonType::idType::hasForeignKey> ));
        BOOST_MPL_ASSERT(( Sql::PersonType::idType::primaryKey ));
        BOOST_MPL_ASSERT(( Sql::PersonType::idType::notNull ));
        BOOST_MPL_ASSERT(( boost::mpl::not_<Sql::PersonType::PersonForenameType::notNull> ));
        BOOST_MPL_ASSERT(( boost::mpl::not_<Sql::PersonType::PersonForenameType::primaryKey> ));
        BOOST_MPL_ASSERT(( boost::mpl::not_<Sql::PersonType::PersonForenameType::unique> ));
        BOOST_MPL_ASSERT(( Sql::PersonType::PersonGradeType::hasForeignKey ));
    }

    void testColumnAliasing()
    {
        BOOST_MPL_ASSERT(( boost::is_same<Sql::PersonSubRolesRelationType::leftType, Sql::PersonSubRolesRelationType::fk_tblPerson_idType> ));
        QCOMPARE( Sql::PersonSubRolesRelation.left.name(), Sql::PersonSubRolesRelation.fk_tblPerson_id.name() );
    }
};

QTEST_MAIN( SchemaTest )

#include "schematest.moc"
