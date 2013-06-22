# Generated files from folly/build/generate_*.py
auto_sources(genfiles "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/gen")

# Main folly library files
auto_sources(files "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/folly")
auto_sources(cfiles "*.c" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/folly")

# most folly tests require linux-specific functionality
foreach (file ${files})
    if (APPLE)
        if (${file} MATCHES "/folly/(.*)/test/")
            list(REMOVE_ITEM files ${file})
        endif()
    endif()
endforeach()

# remove additional non-portable items
if (APPLE)
    list(REMOVE_ITEM files ${CMAKE_CURRENT_SOURCE_DIR}/folly/Benchmark.cpp)
    list(REMOVE_ITEM files ${CMAKE_CURRENT_SOURCE_DIR}/folly/experimental/exception_tracer/ExceptionTracerTest.cpp)
    list(REMOVE_ITEM files ${CMAKE_CURRENT_SOURCE_DIR}/folly/experimental/exception_tracer/ExceptionTracer.cpp)
    list(REMOVE_ITEM files ${CMAKE_CURRENT_SOURCE_DIR}/folly/experimental/exception_tracer/ExceptionTracerLib.cpp)
    list(REMOVE_ITEM files ${CMAKE_CURRENT_SOURCE_DIR}/folly/experimental/io/AsyncIO.cpp)
    list(REMOVE_ITEM files ${CMAKE_CURRENT_SOURCE_DIR}/folly/experimental/symbolizer/Dwarf.cpp)
    list(REMOVE_ITEM files ${CMAKE_CURRENT_SOURCE_DIR}/folly/experimental/symbolizer/Elf.cpp)
    list(REMOVE_ITEM files ${CMAKE_CURRENT_SOURCE_DIR}/folly/experimental/symbolizer/ElfUtil.cpp)
    list(REMOVE_ITEM files ${CMAKE_CURRENT_SOURCE_DIR}/folly/experimental/symbolizer/Symbolizer.cpp)
    list(REMOVE_ITEM files ${CMAKE_CURRENT_SOURCE_DIR}/folly/experimental/symbolizer/SymbolizerTest.cpp)
    list(REMOVE_ITEM files ${CMAKE_CURRENT_SOURCE_DIR}/folly/experimental/symbolizer/test/ElfTests.cpp)
endif()

# Subprocess requires flatmap from boost 1.48
# and some other folly pieces we're not including yet
# For now, that's not actually a requirement, so skip it
list(REMOVE_ITEM files ${CMAKE_CURRENT_SOURCE_DIR}/folly/Subprocess.cpp)
add_library(folly STATIC ${files} ${genfiles} ${cfiles} )

find_package(Boost 1.48.0 COMPONENTS system program_options filesystem regex REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

find_package(Glog REQUIRED)
include_directories(${LIBGLOG_INCLUDE_DIR})

find_package(PThread REQUIRED)
include_directories(${LIBPTHREAD_INCLUDE_DIRS})

target_link_libraries(folly ${Boost_LIBRARIES} ${LIBGLOG_LIBRARY} ${LIBPTHREAD_LIBRARIES} ${JEMALLOC_LIB})
