# SQLATE

Sqlate is a C++ library to interact with SQL databases that uses. It uses [Qt][https://www.qt.io] and advanced C++ templates
that helps to write SQL queries in a type safe way, checked at compile time.

## Main features

### Table definitions
* define tables as C++ structures
* use C++ types to identify columns
* get the SQL name from the C++ structure

```c++
TABLE( Presenters) {
    SQL_NAME( "presenters" );
    COLUMN( name, QString, NotNull );
    COLUMN( age, int, NotNull );
    COLUMN( company, QString);
    typedef boost::mpl::vector<nameType, ageType, companyType> columns;
};
```

### Building queries
* no need to refer to table and column names as strings
* compile time checks for correctness

#### Query builders for INSERT, SELECT, DELETE, UPDATE, CREATE TABLE

```c++
SqlSelectQueryBuilder qb;
qb.setTable(Presenters);
qb.addColumn(Presenters.nam);
qb.whereCondition().addValueCondition( Presenters.age, SqlCondition::Equals, 42);
qb.whereCondition().addValueCondition( Presenters.company, SqlCondition::Equals,  “KDAB”);
QSqlQuery query = qb.exec();
```

#### Natural SQL syntax in C++ for SELECT, INSERT (more to follow)

```c++
QSqlQuery query = select(Presenters.name).from(Presenters).where(Presenters.age == 42 && Presenters.company == “KDAB”);
...
QSqlQuery insertQuery = insert().into(Presenters).columns(Presenters.age << 22 & Presenters.company << "KDAB");
```

### Other features
* support complex join statements
* transaction and monitor support
* support for schema updates
* extensible to support multiple database type and hides the SQL dialect difference from the end user (currently only PostgreSQL
support is implemented)
* exception based error handling


## Prerequisites

* CMake 3.2.0 or above
* Qt 5.4 or above
* Boost 1.40 or newer
* Compiler supporting C++11/14
* PostgreSQL (runtime dependency for unit tests)

## Building from source

### Linux
```sh
git clone git@github.com:KDAB/sqlate.git
cd sqlate
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=dest_dir
make
sudo make install
```

### OSX, Windows and Android
* the Qt 5 version was not tested on these systems as this moment. The Qt4 version works on these platforms, on Windows requiring
MSVC 2010 or newer. For all systems the PostgreSQL Qt driver must be built and installed and Boost needs to be available.
The Qt4 version can be found by checking out the qt4 branch.

### Unit tests

To run unit tests use
```sh
ctest
```

in the build directory. For details about the unit test prerequisites, read the README.txt file in the tests directory.

## License
* LGPL v2.1

Sqlate is Copyright (C) 2011-2017 Klaralvdalens Datakonsult AB.

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
