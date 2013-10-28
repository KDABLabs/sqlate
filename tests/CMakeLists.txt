site_name(HOSTNAME)
if (CMAKE_VERSION VERSION_LESS 2.8.11)
  string(REPLACE "-" "_" HOSTNAME ${HOSTNAME})
  string(REPLACE "." "_" HOSTNAME ${HOSTNAME})
else()
  string(MAKE_C_IDENTIFIER ${HOSTNAME} HOSTNAME)
endif()

include_directories(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/foo
)

if (NOT SQLATE_DATABASE_HOST)
  set(SQLATE_DATABASE_HOST "sqlate-postgres")
endif()

if (NOT SQLATE_DATABASE_USER)
  set(SQLATE_DATABASE_USER "sqlate.user")
endif()

if (NOT SQLATE_DATABASE_PASSWORD)
  set(SQLATE_DATABASE_PASSWORD "sql123456")
endif()


set(testcommon_srcs testbase.cpp testschema.cpp )
add_library(testcommon STATIC ${testcommon_srcs})

macro(add_sql_unittest _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)

  add_executable(${_name} ${_source})
  add_test(NAME sql-${_name} COMMAND ${_name})
  target_link_libraries(${_name} sqlate testcommon Qt4::QtTest)
endmacro()

#macro for test needing db access (they derive from TestBase)
macro(add_sql_unittest_testbase _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)
  add_executable(${_name} ${_source})

  add_test(NAME sql-${_name} COMMAND ${_name})
  set_tests_properties(sql-${_name} PROPERTIES ENVIRONMENT
    "SQLATE_DATABASE_HOST=${SQLATE_DATABASE_HOST};SQLATE_DATABASE_NAME=sqlate_test_${HOSTNAME};SQLATE_DATABASE_USER=${SQLATE_DATABASE_USER};SQLATE_DATABASE_PASSWORD=${SQLATE_DATABASE_PASSWORD}"
  )
  target_link_libraries(${_name} Qt4::QtTest
      sqlate_schemaupdate testcommon)
endmacro()

add_sql_unittest(schematest.cpp)
add_sql_unittest(createtabletest.cpp)
add_sql_unittest(createruletest.cpp)
add_sql_unittest(sqlutilstest.cpp)

add_sql_unittest_testbase(selectquerybuildertest.cpp)
add_sql_unittest_testbase(insertquerybuildertest.cpp)
add_sql_unittest_testbase(updatequerybuildertest.cpp)
add_sql_unittest_testbase(deletequerybuildertest.cpp)
add_sql_unittest_testbase(selecttest.cpp)
add_sql_unittest_testbase(inserttest.cpp)
add_sql_unittest_testbase(schemaupdatetest.cpp)
