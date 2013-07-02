project(sqlatelib)
set(SQLATE_LIBRARY_VERSION 0)
set(SQLATE_LIBRARY_SO_VERSION 1)

cmake_minimum_required(VERSION 2.8)
set(CMAKE_AUTOMOC true)
option(SQL_ENABLE_NETWORK_WATCHER "Enable the watcher threads for SQL queries over the network. The default is disabled." FALSE)

if (SQL_ENABLE_NETWORK_WATCHER)
    add_definitions(-DSQL_ENABLE_NETWORK_WATCHER)
endif ()

if (ANDROID)
  find_host_package(Qt4 REQUIRED)
  include(Android)
else (ANDROID)
  find_package(Qt4 REQUIRED)
endif()

find_package(Boost 1.40 REQUIRED)

#the user of the library might have defined a larger size
set(MPL_LIMIT_DEFINED "FALSE")
get_directory_property( DirDefs DIRECTORY ${CMAKE_SOURCE_DIR} COMPILE_DEFINITIONS )
foreach(define ${DirDefs})
    if (${define} MATCHES "BOOST_MPL_LIMIT_VECTOR_SIZE=.*")
        set(MPL_LIMIT_DEFINED "TRUE")
    endif()
endforeach()
if (NOT ${MPL_LIMIT_DEFINED})
    add_definitions( -DBOOST_MPL_LIMIT_VECTOR_SIZE=50)
endif()

add_definitions( -DBOOST_MPL_CFG_NO_PREPROCESSED_HEADERS )
add_definitions( -DQT_STRICT_ITERATORS )
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )
add_definitions( -DQT_NO_CAST_FROM_BYTEARRAY )
add_definitions( -DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

enable_testing()

if(APPLE)
    set(BIN_INSTALL_DIR ".")
    # No LIB_INSTALL_DIR on APPLE, we use BundleUtilities instead.
    # No PLUGIN_INSTALL_DIR on APPLE, we use BundleUtilities instead.
    # If the install directory is the default then set to a child dir
    # of the the binary install. Otherwise we assume the user has specified
    # a CMAKE_INSTALL_PREFIX define
    # FIXME: there must be a better way of detecting the default vs user set
    if(CMAKE_INSTALL_PREFIX MATCHES "/usr/local")
        set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/bin")
    endif()
elseif(WIN32)
    set(BIN_INSTALL_DIR ".")
    set(LIB_INSTALL_DIR ".")
    set(PLUGIN_INSTALL_DIR "plugins")
elseif(NOT ANDROID)
    set(BIN_INSTALL_DIR "bin")
    set(LIB_SUFFIX "" CACHE STRING "Define suffix of directory name (32/64)")
    set(LIB_INSTALL_DIR "lib${LIB_SUFFIX}")
    set(PLUGIN_INSTALL_DIR "${LIB_INSTALL_DIR}/plugins")
endif()

include_directories( ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
    ${QT_INCLUDES})

set(SQLATE_SRCS
    kdthreadrunner.cpp
    SqlCondition.cpp
    SqlQuery.cpp
    SqlQueryBuilderBase.cpp
    SqlConditionalQueryBuilderBase.cpp
    SqlDeleteQueryBuilder.cpp
    SqlSelectQueryBuilder.cpp
    SqlInsertQueryBuilder.cpp
    SqlUpdateQueryBuilder.cpp
    SqlCreateTable.cpp
    SqlSchema.cpp
    SqlTransaction.cpp
    SqlMonitor.cpp
    SqlQueryManager.cpp
    SqlQueryWatcher.cpp
    SqlUtils.cpp
    PostgresSchema.cpp
    SchemaUpdater.cpp
    SqlQueryCache.cpp
)

qt4_add_resources(sql_resources SqlResources.qrc )

add_library(sqlate SHARED ${SQLATE_SRCS} ${sql_resources})

if (NOT ANDROID)
set_target_properties(sqlate PROPERTIES
        VERSION ${SQLATE_LIBRARY_VERSION}
        SOVERSION ${SQLATE_LIBRARY_SO_VERSION}
)
endif()

set_target_properties(sqlate PROPERTIES
        DEFINE_SYMBOL SQLATE_BUILD_SQLATE_LIB
)

target_link_libraries(sqlate ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTSQL_LIBRARY})

if(NOT APPLE AND NOT ANDROID)
    install(TARGETS sqlate RUNTIME DESTINATION ${BIN_INSTALL_DIR} LIBRARY DESTINATION ${LIB_INSTALL_DIR})
endif()

add_executable(sqlschema2dot sqlschema2dot.cpp)
target_link_libraries(sqlschema2dot  ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTSQL_LIBRARY} sqlate)

add_custom_target(schema-graph
    ${CMAKE_CURRENT_BINARY_DIR}/sqlschema2dot > sqlschema.dot &&
    dot -Tpng sqlschema.dot > sqlschema.png
)

add_subdirectory(tests)
