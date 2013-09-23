function(auto_sources RETURN_VALUE PATTERN SOURCE_SUBDIRS)

	if ("${SOURCE_SUBDIRS}" STREQUAL "RECURSE")
		SET(PATH ".")
		if (${ARGC} EQUAL 4)
			list(GET ARGV 3 PATH)
		endif ()
	endif()

	if ("${SOURCE_SUBDIRS}" STREQUAL "RECURSE")
		unset(${RETURN_VALUE})
		file(GLOB SUBDIR_FILES "${PATH}/${PATTERN}")
		list(APPEND ${RETURN_VALUE} ${SUBDIR_FILES})

		file(GLOB subdirs RELATIVE ${PATH} ${PATH}/*)

		foreach(DIR ${subdirs})
			if (IS_DIRECTORY ${PATH}/${DIR})
				if (NOT "${DIR}" STREQUAL "CMakeFiles")
					file(GLOB_RECURSE SUBDIR_FILES "${PATH}/${DIR}/${PATTERN}")
					list(APPEND ${RETURN_VALUE} ${SUBDIR_FILES})
				endif()
			endif()
		endforeach()
	else ()
		file(GLOB ${RETURN_VALUE} "${PATTERN}")

		foreach (PATH ${SOURCE_SUBDIRS})
			file(GLOB SUBDIR_FILES "${PATH}/${PATTERN}")
			list(APPEND ${RETURN_VALUE} ${SUBDIR_FILES})
		endforeach(PATH ${SOURCE_SUBDIRS})
	endif ()

	if (${FILTER_OUT})
		list(REMOVE_ITEM ${RETURN_VALUE} ${FILTER_OUT})
	endif()

	set(${RETURN_VALUE} ${${RETURN_VALUE}} PARENT_SCOPE)
endfunction(auto_sources)

function(CONTAINS_STRING FILE SEARCH RETURN_VALUE)
	file(STRINGS ${FILE} FILE_CONTENTS REGEX ".*${SEARCH}.*")
	if (FILE_CONTENTS)
		set(${RETURN_VALUE} True PARENT_SCOPE)
	endif()
endfunction(CONTAINS_STRING)

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

function(embed_systemlib TARGET DEST SOURCE)
	if (APPLE)
	        target_link_libraries(${TARGET} -Wl,-sectcreate,__text,systemlib,${SOURCE})
	else()
	        add_custom_command(TARGET ${TARGET} POST_BUILD
	                   COMMAND "objcopy"
	                   ARGS "--add-section" "systemlib=${SOURCE}" ${DEST}
	                   COMMENT "Embedding systemlib.php in ${TARGET}")
	endif()
endfunction(embed_systemlib)
