INCLUDE(CMakeDependentOption)
CMAKE_DEPENDENT_OPTION(ENABLE_MONGO
  "Enable Mongo extension (requires ENABLE_ZEND_COMPAT)" OFF
  "ENABLE_ZEND_COMPAT" OFF)
CMAKE_DEPENDENT_OPTION(ENABLE_EZC_TEST
  "Enable ezc_test (requires ENABLE_ZEND_COMPAT)" OFF "ENABLE_ZEND_COMPAT" OFF)

set(CXX_SOURCES)
set(C_SOURCES)
set(ASM_SOURCES)
set(HEADER_SOURCES)
set(ZEND_COMPAT_LINK_LIBRARIES)
set(PHP_SOURCES)
set(EZC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
HHVM_SELECT_SOURCES("${EZC_DIR}/php-src")
HHVM_SELECT_SOURCES("${EZC_DIR}/hhvm")

include_directories("${EZC_DIR}/php-src")
include_directories("${EZC_DIR}/php-src/main")
include_directories("${EZC_DIR}/php-src/Zend")
include_directories("${EZC_DIR}/php-src/TSRM")

if (ENABLE_ZEND_COMPAT)
  macro(auto_systemlib DIR)
    auto_sources(files "*.php" RECURSE ${DIR})
    foreach(f ${files})
      list(APPEND PHP_SOURCES ${f})
    endforeach()
  endmacro()

  set(ZEND_COMPAT_PROJECTS)
  # Look for projects
  file(GLOB ezc_projects RELATIVE ${EZC_DIR} "${EZC_DIR}/*")
  foreach(ezc_project ${ezc_projects})
    get_filename_component(ezc_name ${ezc_project} NAME)
    #
    # The subdirectory dso_test is built using hphpize,
    # and is NOT statically linked into hhvm.
    # It is intended to be loaded as a DSO during test.
    #
    if ((NOT ${ezc_name} STREQUAL "dso_test") AND
        (NOT ${ezc_name} STREQUAL "php-src") AND
        (NOT ${ezc_name} STREQUAL "hhvm") AND
        (IS_DIRECTORY "${EZC_DIR}/${ezc_name}"))
      list(APPEND ZEND_COMPAT_PROJECTS ${ezc_name})
    endif()
  endforeach()

  foreach(ezc_project ${ZEND_COMPAT_PROJECTS})
    if (${ezc_project} STREQUAL "yaml")
      find_package(LibYaml)
      if (LibYaml_INCLUDE_DIRS)
        HHVM_SELECT_SOURCES("${EZC_DIR}/yaml")
        include_directories(${LibYaml_INCLUDE_DIRS})
        list(APPEND ZEND_COMPAT_LINK_LIBRARIES ${LibYaml_LIBRARIES})
        auto_systemlib("${EZC_DIR}/yaml")
      endif()
    elseif (${ezc_project} STREQUAL "mongo")
      if (ENABLE_MONGO)
        message(WARNING "The ext_zend_compat version of the MongoDB extension is deprecated. Please use the official driver at https://github.com/mongodb/mongo-hhvm-driver instead.")
        HHVM_SELECT_SOURCES("${EZC_DIR}/mongo")
        include_directories("${EZC_DIR}/mongo/mcon")
        auto_systemlib("${EZC_DIR}/mongo")
      endif()
    elseif (${ezc_project} STREQUAL "ezc_test")
      if (ENABLE_EZC_TEST)
        HHVM_SELECT_SOURCES("${EZC_DIR}/ezc_test")
        auto_systemlib("${EZC_DIR}/ezc_test")
      endif()
    else()
      HHVM_SELECT_SOURCES("${EZC_DIR}/${ezc_project}")
      auto_systemlib("${EZC_DIR}/${ezc_project}")
    endif()
  endforeach()

endif()
set(EZC_SYSTEMLIB_SOURCES ${PHP_SOURCES} CACHE INTERNAL "" FORCE)

add_library(hphp_ext_zend_compat STATIC ${CXX_SOURCES} ${C_SOURCES}
            ${ASM_SOURCES} ${HEADER_SOURCES})
auto_source_group("hphp_ext_zend_compat" "${CMAKE_CURRENT_SOURCE_DIR}"
  ${CXX_SOURCES} ${C_SOURCES} ${ASM_SOURCES} ${HEADER_SOURCES})
if (ZEND_COMPAT_LINK_LIBRARIES)
  target_link_libraries(hphp_ext_zend_compat ${ZEND_COMPAT_LINK_LIBRARIES})
endif()
hphp_link(hphp_ext_zend_compat)

HHVM_PUBLIC_HEADERS(ext_zend_compat ${HEADER_SOURCES})

include(dso_test/dso_test.cmake)
