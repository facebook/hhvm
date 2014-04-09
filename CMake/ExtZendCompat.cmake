option(ENABLE_ZEND_COMPAT "Enable Zend source compatibility extensions" OFF)
INCLUDE(CMakeDependentOption)
CMAKE_DEPENDENT_OPTION(ENABLE_MONGO
  "Enable Mongo extension (requires ENABLE_ZEND_COMPAT)" OFF "ENABLE_ZEND_COMPAT" OFF)
CMAKE_DEPENDENT_OPTION(ENABLE_EZC_TEST
  "Enable ezc_test (requires ENABLE_ZEND_COMPAT)" OFF "ENABLE_ZEND_COMPAT" OFF)

set(ZEND_COMPAT_PROJECTS)
set(ZEND_COMPAT_BUILD_DIRS)
set(ZEND_COMPAT_LINK_LIBRARIES)
set(EZC_DIR "${HPHP_HOME}/hphp/runtime/ext_zend_compat/")

if (ENABLE_ZEND_COMPAT)
	# Look for projects
	file(GLOB ezc_projects RELATIVE ${EZC_DIR} "${EZC_DIR}/*")
	foreach(ezc_project ${ezc_projects})
		get_filename_component(ezc_name ${ezc_project} NAME)
		if ((NOT ${ezc_name} STREQUAL "php-src") AND
	            (NOT ${ezc_name} STREQUAL "hhvm") AND
	            (IS_DIRECTORY "${EZC_DIR}/${ezc_name}"))
			list(APPEND ZEND_COMPAT_PROJECTS ${ezc_name})
		endif()
	endforeach()

	foreach(ezc_project ${ZEND_COMPAT_PROJECTS})
		if (${ezc_project} STREQUAL "yaml")
			find_package(LibYaml)
			if (LibYaml_INCLUDE_DIRS)
				list(APPEND ZEND_COMPAT_BUILD_DIRS "${EZC_DIR}/yaml")
				include_directories(${LibYaml_INCLUDE_DIRS})
				list(APPEND ZEND_COMPAT_LINK_LIBRARIES ${LibYaml_LIBRARIES})
			endif()
		elseif (${ezc_project} STREQUAL "mongo")
			if (ENABLE_MONGO)
				include_directories("${EZC_DIR}/mongo/mcon")
				list(APPEND ZEND_COMPAT_BUILD_DIRS "${EZC_DIR}/mongo")
			endif()
		elseif (${ezc_project} STREQUAL "ezc_test")
			if (ENABLE_EZC_TEST)
				list(APPEND ZEND_COMPAT_BUILD_DIRS "${EZC_DIR}/${ezc_project}")
			endif()
		else()
			list(APPEND ZEND_COMPAT_BUILD_DIRS "${EZC_DIR}/${ezc_project}")
		endif()
	endforeach()
endif()

list(APPEND ZEND_COMPAT_BUILD_DIRS "${EZC_DIR}/php-src" "${EZC_DIR}/hhvm")
include_directories("${EZC_DIR}/php-src")
include_directories("${EZC_DIR}/php-src/main")
include_directories("${EZC_DIR}/php-src/Zend")
include_directories("${EZC_DIR}/php-src/TSRM")

