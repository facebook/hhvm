function(auto_sources RETURN_VALUE PATTERN SOURCE_SUBDIRS)

	if ("${SOURCE_SUBDIRS}" STREQUAL "RECURSE")
		SET(PATH ".")
		if (${ARGC} EQUAL 4)
			list(GET ARGV 3 PATH)
		endif (${ARGC} EQUAL 4)
	endif("${SOURCE_SUBDIRS}" STREQUAL "RECURSE")

	if ("${SOURCE_SUBDIRS}" STREQUAL "RECURSE")

		file(GLOB_RECURSE ${RETURN_VALUE} "${PATH}/${PATTERN}")

	else ("${SOURCE_SUBDIRS}" STREQUAL "RECURSE")

		file(GLOB ${RETURN_VALUE} "${PATTERN}")

		foreach (PATH ${SOURCE_SUBDIRS})
			file(GLOB SUBDIR_FILES "${PATH}/${PATTERN}")
			list(APPEND ${RETURN_VALUE} ${SUBDIR_FILES})
		endforeach(PATH ${SOURCE_SUBDIRS})

	endif ("${SOURCE_SUBDIRS}" STREQUAL "RECURSE")

	if (${FILTER_OUT})
		list(REMOVE_ITEM ${RETURN_VALUE} ${FILTER_OUT})
	endif(${FILTER_OUT})

	set(${RETURN_VALUE} ${${RETURN_VALUE}} PARENT_SCOPE)
  
endfunction(auto_sources)

macro(MYSQL_SOCKET_SEARCH)
	foreach (i
		/var/run/mysqld/mysqld.sock
		/var/tmp/mysql.sock
		/var/run/mysql/mysql.sock
		/var/lib/mysql/mysql.sock
		/var/mysql/mysql.sock
		/usr/local/mysql/var/mysql.sock
		/Private/tmp/mysql.sock
		/private/tmp/mysql.sock
		/tmp/mysql.sock
		)
		if (EXISTS ${i})
			set(MYSQL_SOCK ${i})
			break()
		endif()
	endforeach()

	if (MYSQL_SOCK)
		set(MYSQL_UNIX_SOCK_ADDR ${MYSQL_SOCK} CACHE STRING "Path to MySQL Socket")	
	endif()
endmacro()
