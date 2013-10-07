IF(PGSQL_FOUND)
  SET(PGSQL_FIND_QUIETLY TRUE)
ENDIF(PGSQL_FOUND)

# Find include directory
FIND_PATH(PGSQL_INCLUDE_DIR NAMES libpq-fe.h
    PATHS /usr/include /usr/include/postgresql /usr/local/include /usr/local/include/postgresql)

FIND_LIBRARY(PGSQL_LIBRARY NAMES pq PATHS /lib /usr/lib /usr/local/lib)

IF (PGSQL_INCLUDE_DIR AND PGSQL_LIBRARY)
    SET(PGSQL_FOUND TRUE)
	MESSAGE(STATUS "pgSQL Include dir: ${PGSQL_INCLUDE_DIR}")
	MESSAGE(STATUS "libpq library: ${PGSQL_LIBRARY}")
ENDIF()

MARK_AS_ADVANCED(
    PGSQL_INCLUDE_DIR
    PGSQL_FOUND
    PGSQL_LIBRARY
)
