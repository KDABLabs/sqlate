site_name(HOSTNAME)
string(REPLACE "-" "_" HOSTNAME ${HOSTNAME})
string(REPLACE "." "_" HOSTNAME ${HOSTNAME})

include_directories(${CMAKE_BINARY_DIR}/tests ${CMAKE_SOURCE_DIR})

set(testcommon_srcs testbase.cpp testschema.cpp )
add_library(testcommon STATIC ${testcommon_srcs})

macro(add_sql_unittest _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)

  add_executable(${_name} ${_source})
  add_test(NAME sql-${_name} COMMAND ${_name})
  target_link_libraries(${_name} sqlate testcommon ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTTEST_LIBRARY} ${QT_QTSQL_LIBRARY})
endmacro()

#macro for test needing db access (they derive from TestBase)
macro(add_sql_unittest_testbase _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)
  add_executable(${_name} ${_source})

  add_test(NAME sql-${_name} COMMAND ${_name})
  set_tests_properties(sql-${_name} PROPERTIES ENVIRONMENT "SQLATE_DATABASE_HOST=sqlate-postgres;SQLATE_DATABASE_NAME=sqlate_test_${HOSTNAME}")
  target_link_libraries(${_name} ${QT_QTCORE_LIBRARY} ${QT_QTTEST_LIBRARY}
      ${QT_QTSQL_LIBRARY} sqlate testcommon)
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
add_sql_unittest_testbase(schemaupdatetest.cpp)
