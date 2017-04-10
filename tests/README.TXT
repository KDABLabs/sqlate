In order to run the unit tests some steps must be followed:

1) a PostgreSQL server must be running and needs to be accessible via TCP under the "sqlate-postgres" name.
If it is not possible to use this domain name, the SQLATE_DATABASE_HOST environment variable should contain the name or address of the PostgreSQL server instance.

2) the server must have two group roles: "sqladmins" and "sqlusers". The admin group should have database
create rights.

3) the server must have a user called "sqlate.user" with password "sql123456". The user must be in the
"sqladmins" and "postgres" groups and should inherit group rights. Should you want to use a different user
or password, you can override the default values with SQLATE_DATABASE_USER and SQLATE_DATABASE_PASSWORD
environment variables.
The user should be able to log in with "password" method.


When run with ctest, the tests will create a database called "sqlate_test_HOSTNAME" where HOSTNAME is
the name of the machine where the tests are run.

If the tests are run individually from command line or from Qt Creator, they will create an "sqlate_test"
database, unless a database name is specified via the SQLATE_DATABASE_NAME environment variable.
When run individually, the tests will try to contact the PostgreSQL server on localhost, specify the
server name in the SQLATE_DATABASE_HOST if needed.
