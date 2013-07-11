# folly-config.h is a generated file from autotools
# We need to do the equivalent checks here and use
# add_definitions as needed
add_definitions(-DFOLLY_NO_CONFIG=1)

set(FOLLY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/folly")

# Generated files from folly/build/generate_*.py
auto_sources(genfiles "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/gen")

# Main folly library files
auto_sources(files "*.cpp" "RECURSE" "${FOLLY_DIR}")
auto_sources(cfiles "*.c" "RECURSE" "${FOLLY_DIR}")

# No need for tests, Benchmarks, Utils, or most experimental stuff
foreach (file ${files})
	if (${file} MATCHES "/test/")
		list(REMOVE_ITEM files ${file})
	endif()
endforeach()
list(REMOVE_ITEM files
	${FOLLY_DIR}/Benchmark.cpp
	${FOLLY_DIR}/experimental/File.cpp
	${FOLLY_DIR}/experimental/exception_tracer/ExceptionTracer.cpp
	${FOLLY_DIR}/experimental/exception_tracer/ExceptionTracerLib.cpp
	${FOLLY_DIR}/experimental/exception_tracer/ExceptionTracerTest.cpp
	${FOLLY_DIR}/experimental/io/AsyncIO.cpp
	${FOLLY_DIR}/experimental/symbolizer/SymbolizerTest.cpp
	${FOLLY_DIR}/experimental/symbolizer/ElfUtil.cpp
    )

# Remove non-portable items
if (NOT LINUX)
	list(REMOVE_ITEM files ${FOLLY_DIR}/experimental/symbolizer/Symbolizer.cpp)
	list(REMOVE_ITEM files ${FOLLY_DIR}/experimental/symbolizer/Dwarf.cpp)
	list(REMOVE_ITEM files ${FOLLY_DIR}/experimental/symbolizer/Elf.cpp)
endif()

# Subprocess requires flatmap from boost 1.48
# and some other folly pieces we're not including yet
# For now, that's not actually a requirement, so skip it
list(REMOVE_ITEM files ${FOLLY_DIR}/Subprocess.cpp)
add_library(folly STATIC ${files} ${genfiles} ${cfiles} )

find_package(Boost 1.48.0 COMPONENTS system program_options filesystem regex REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

find_package(Glog REQUIRED)
include_directories(${LIBGLOG_INCLUDE_DIR})

find_package(PThread REQUIRED)
include_directories(${LIBPTHREAD_INCLUDE_DIRS})

find_package(Libunwind REQUIRED)
include_directories(${LIBUNWIND_INCLUDE_DIR})

INCLUDE(CheckCXXSourceCompiles)
CHECK_CXX_SOURCE_COMPILES("
extern \"C\" void (*test_ifunc(void))() { return 0; }
void func() __attribute__((ifunc(\"test_ifunc\")));
" FOLLY_IFUNC)
if (FOLLY_IFUNC)
	add_definitions("-DHAVE_IFUNC=1")
endif()

target_link_libraries(folly ${Boost_LIBRARIES} ${LIBGLOG_LIBRARY} ${LIBPTHREAD_LIBRARIES} ${LIBUNWIND_LIBRARY})
