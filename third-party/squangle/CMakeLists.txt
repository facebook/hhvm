set(SQUANGLE_INCLUDE_ROOT "")
set(SQUANGLE_SOURCE_ROOT "")
foreach(SQUANGLE_SOURCE_CANDIDATE
        "${CMAKE_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/src")
  if(EXISTS "${SQUANGLE_SOURCE_CANDIDATE}/squangle/mysql_client/Row.h")
    set(SQUANGLE_INCLUDE_ROOT "${SQUANGLE_SOURCE_CANDIDATE}")
    set(SQUANGLE_SOURCE_ROOT "${SQUANGLE_SOURCE_CANDIDATE}/squangle")
    break()
  endif()
endforeach()

if(NOT SQUANGLE_SOURCE_ROOT)
  message(FATAL_ERROR
    "Squangle source not found. Populate the assembled OSS tree with "
    "third-party/squangle/src/squangle (or the root squangle symlink) instead "
    "of silently disabling a required dependency."
  )
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  execute_process(
    COMMAND bash "${CMAKE_CURRENT_SOURCE_DIR}/apply-gcc-patches.sh" "${SQUANGLE_INCLUDE_ROOT}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    RESULT_VARIABLE SQUANGLE_PATCH_RESULT
  )
  if(NOT SQUANGLE_PATCH_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to apply GCC compatibility patches to squangle")
  endif()
endif()

file(GLOB_RECURSE SQUANGLE_SOURCES "${SQUANGLE_SOURCE_ROOT}/*.cpp")
foreach(src ${SQUANGLE_SOURCES})
  if(src MATCHES "/(test|tests|facebook)/")
    list(REMOVE_ITEM SQUANGLE_SOURCES "${src}")
  endif()
endforeach()

add_library(squangle STATIC ${SQUANGLE_SOURCES})
set_target_properties(squangle PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(squangle PUBLIC "${SQUANGLE_INCLUDE_ROOT}")
target_link_libraries(squangle PUBLIC fbmysqlclient Folly::folly re2)

