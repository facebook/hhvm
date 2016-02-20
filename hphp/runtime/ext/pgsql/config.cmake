HHVM_DEFINE_EXTENSION("pgsql" IMPLICIT
  SOURCES
    pgsql.cpp
    pdo_pgsql_statement.cpp
    pdo_pgsql_connection.cpp
    pdo_pgsql.cpp
  HEADERS
    pdo_pgsql_connection.h
    pdo_pgsql.h
    pdo_pgsql_resource.h
    pdo_pgsql_statement.h
  SYSTEMLIB
    ext_pgsql.php
  DEPENDS
    libPgSQL
)
