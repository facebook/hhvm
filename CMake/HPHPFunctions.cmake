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

macro(HHVM_SELECT_SOURCES DIR)
	auto_sources(files "*.cpp" "RECURSE" "${DIR}")
	foreach(f ${files})
		if (NOT (${f} MATCHES "(ext_hhvm|/(old-)?tests?/)"))
			list(APPEND CXX_SOURCES ${f})
		endif()
	endforeach()
	auto_sources(files "*.c" "RECURSE" "${DIR}")
	foreach(f ${files})
		if (NOT (${f} MATCHES "(ext_hhvm|/(old-)?tests?/)"))
			list(APPEND C_SOURCES ${f})
		endif()
	endforeach()
	auto_sources(files "*.S" "RECURSE" "${DIR}")
	foreach(f ${files})
		if (NOT (${f} MATCHES "(ext_hhvm|/(old-)?tests?/)"))
			list(APPEND ASM_SOURCES ${f})
		endif()
	endforeach()
endmacro(HHVM_SELECT_SOURCES)

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

function(embed_systemlib TARGET DEST SOURCE SECTNAME)
	if (APPLE)
	        target_link_libraries(${TARGET} -Wl,-sectcreate,__text,${SECTNAME},${SOURCE})
	else()
	        add_custom_command(TARGET ${TARGET} POST_BUILD
	                   COMMAND "objcopy"
	                   ARGS "--add-section" "${SECTNAME}=${SOURCE}" ${DEST}
	                   COMMENT "Embedding ${SOURCE} in ${TARGET} as ${SECTNAME}")
	endif()
	# Add the systemlib file to the "LINK_DEPENDS" for the systemlib, this will cause it
	# to be relinked and the systemlib re-embedded
	set_property(TARGET ${TARGET} APPEND PROPERTY LINK_DEPENDS ${SOURCE})
endfunction(embed_systemlib)

function(embed_all_systemlibs TARGET DEST)
	embed_systemlib(${TARGET} ${DEST} ${HPHP_HOME}/hphp/system/systemlib.php systemlib)
	auto_sources(SYSTEMLIBS "ext_*.php" "RECURSE" "${HPHP_HOME}/hphp/runtime")
	foreach(SLIB ${SYSTEMLIBS})
		get_filename_component(SLIB_BN ${SLIB} "NAME_WE")
		string(LENGTH ${SLIB_BN} SLIB_BN_LEN)
		math(EXPR SLIB_BN_REL_LEN "${SLIB_BN_LEN} - 4")
		string(SUBSTRING ${SLIB_BN} 4 ${SLIB_BN_REL_LEN} SLIB_EXTNAME)
		string(MD5 SLIB_HASH_NAME ${SLIB_EXTNAME})
		string(SUBSTRING ${SLIB_HASH_NAME} 0 12 SLIB_HASH_NAME_SHORT)
		embed_systemlib(${TARGET} ${DEST} ${SLIB} "ext.${SLIB_HASH_NAME_SHORT}")
	endforeach()
endfunction(embed_all_systemlibs)

# Custom install function that doesn't relink, instead it uses chrpath to change it, if
# it's available, otherwise, it leaves the chrpath alone
function(HHVM_INSTALL TARGET DEST)
	get_target_property(LOC ${TARGET} LOCATION)
	get_target_property(TY ${TARGET} TYPE)
	if (FOUND_CHRPATH)
		get_target_property(RPATH ${TARGET} INSTALL_RPATH)
		if (NOT RPATH STREQUAL "RPATH-NOTFOUND")
			if (RPATH STREQUAL "")
				install(CODE "execute_process(COMMAND \"${CHRPATH}\" \"-d\" \"${LOC}\" ERROR_QUIET)")
			else()
				install(CODE "execute_process(COMMAND \"${CHRPATH}\" \"-r\" \"${RPATH}\" \"${LOC}\" ERROR_QUIET)")
			endif()
		endif()
	endif()
	install(CODE "FILE(INSTALL DESTINATION \"\${CMAKE_INSTALL_PREFIX}/${DEST}\" TYPE ${TY} FILES \"${LOC}\")")
endfunction(HHVM_INSTALL)

function(HHVM_EXTENSION EXTNAME)
	list(REMOVE_AT ARGV 0)
	add_library(${EXTNAME} SHARED ${ARGV})
	set_target_properties(${EXTNAME} PROPERTIES PREFIX "")
	set_target_properties(${EXTNAME} PROPERTIES SUFFIX ".so")
endfunction()

function(HHVM_SYSTEMLIB EXTNAME SOURCE_FILE)
	embed_systemlib(${EXTNAME} "${EXTNAME}.so" ${SOURCE_FILE} systemlib)
endfunction()
