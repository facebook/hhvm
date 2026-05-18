add_library(libwatchmanclient INTERFACE)

find_package(libWatchmanClient)
if(WATCHMANCLIENT_INCLUDE_DIRS AND WATCHMANCLIENT_LIBRARIES)
  message(STATUS "Using system libwatchmanclient")
  target_include_directories(libwatchmanclient INTERFACE "${WATCHMANCLIENT_INCLUDE_DIRS}")
  target_link_libraries(libwatchmanclient INTERFACE "${WATCHMANCLIENT_LIBRARIES}")
  return()
endif()

set(WATCHMAN_SOURCE_ROOT "")
foreach(WATCHMAN_SOURCE_CANDIDATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src/watchman"
        "${CMAKE_CURRENT_SOURCE_DIR}/src")
  if(EXISTS "${WATCHMAN_SOURCE_CANDIDATE}/cppclient/WatchmanClient.cpp")
    set(WATCHMAN_SOURCE_ROOT "${WATCHMAN_SOURCE_CANDIDATE}")
    break()
  endif()
endforeach()

if(NOT WATCHMAN_SOURCE_ROOT)
  message(STATUS "Watchman client source not found and system libwatchmanclient not available")
  message(STATUS "Extensions requiring watchman will be disabled")
  return()
endif()

set(WATCHMAN_CPPCLIENT_DIR "${WATCHMAN_SOURCE_ROOT}/cppclient")
file(GLOB WATCHMANCLIENT_SOURCES "${WATCHMAN_CPPCLIENT_DIR}/*.cpp")
list(REMOVE_ITEM WATCHMANCLIENT_SOURCES "${WATCHMAN_CPPCLIENT_DIR}/CLI.cpp")

set(WATCHMAN_DEPS folly boost fmt jemalloc double-conversion)
set(WATCHMAN_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")

add_custom_target(
  watchmanclient_include_dir_symlink
  COMMAND ${CMAKE_COMMAND} -E make_directory "${WATCHMAN_INCLUDE_DIR}"
  COMMAND ${CMAKE_COMMAND} -E remove -f "${WATCHMAN_INCLUDE_DIR}/watchman"
  COMMAND
    ${CMAKE_COMMAND} -E create_symlink
    "${WATCHMAN_SOURCE_ROOT}"
    "${WATCHMAN_INCLUDE_DIR}/watchman"
)

add_library(watchmanclient STATIC ${WATCHMANCLIENT_SOURCES})
set_target_properties(watchmanclient PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_dependencies(watchmanclient watchmanclient_include_dir_symlink)
target_include_directories(
  watchmanclient
  PUBLIC
  "${WATCHMAN_CPPCLIENT_DIR}"
  "${WATCHMAN_INCLUDE_DIR}"
)
target_link_libraries(watchmanclient PUBLIC ${WATCHMAN_DEPS})

target_include_directories(libwatchmanclient INTERFACE "${WATCHMAN_INCLUDE_DIR}")
target_link_libraries(libwatchmanclient INTERFACE watchmanclient)
