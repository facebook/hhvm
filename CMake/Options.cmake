#set(CMAKE_BUILD_TYPE Debug)

option(ALWAYS_ASSERT "Enabled asserts in a release build" OFF)
option(DEBUG_MEMORY_LEAK "Allow easier debugging of memory leaks" OFF)
option(DEBUG_APC_LEAK "Allow easier debugging of apc leaks" OFF)
option(STATIC_CXX_LIB "Statically link libstd++ and libgcc." OFF)

option(EXECUTION_PROFILER "Enable the execution profiler" OFF)
option(ENABLE_FULL_SETLINE "Enable full setline function for debugger and code-coverage" OFF)

option(USE_JEMALLOC "Use jemalloc" ON)

option(USE_TCMALLOC "Use tcmalloc (if jemalloc is not used)" ON)
option(USE_GOOGLE_HEAP_PROFILER "Use Google heap profiler" OFF)
option(USE_GOOGLE_CPU_PROFILER "Use Google cpu profiler" OFF)

option(DISABLE_HARDWARE_COUNTERS "Disable hardware counters (for XenU systems)" OFF)

option(ENABLE_TRACE "Enable tracing in release build" OFF)
option(CPACK_GENERATOR "Enable build of distribution packages using CPack" OFF)

IF (NOT APPLE)
  option(ENABLE_ZEND_COMPAT "Enable Zend source compatibility" ON)
ENDIF (NOT APPLE)

option(ENABLE_COTIRE "Speed up the build by precompiling headers" OFF)

option(PACKED_TV "Enable packed tv (typed value) compilation" OFF)
