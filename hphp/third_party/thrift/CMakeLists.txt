set(CXX_SOURCES)
auto_sources(files "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CXX_SOURCES ${files})

find_package(Boost 1.48.0 COMPONENTS thread system REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

include_directories("${HPHP_HOME}/hphp/third_party")
add_library(hphp_thrift STATIC ${CXX_SOURCES})

target_link_libraries(hphp_thrift ${Boost_LIBRARIES}
                                  ${LIBGLOG_LIBRARY}
                                  ${LIBPTHREAD_LIBRARIES})

# Workaround for GCC bug, remove after 4.8 is the minimum supported
add_definitions(-D_GLIBCXX_USE_NANOSLEEP)
