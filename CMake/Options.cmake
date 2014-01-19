#set(CMAKE_BUILD_TYPE Debug)

option(ALWAYS_ASSERT "Enabled asserts in a release build" OFF)
option(DEBUG_MEMORY_LEAK "Allow easier debugging of memory leaks" OFF)
option(DEBUG_APC_LEAK "Allow easier debugging of apc leaks" OFF)

option(HOTPROFILER "Enable support for the hot-profiler" OFF)
option(EXECUTION_PROFILER "Enable the execution profiler" OFF)
option(ENABLE_FULL_SETLINE "Enable full setline function for debugger and code-coverage" OFF)

option(USE_JEMALLOC "Use jemalloc" ON)

option(USE_TCMALLOC "Use tcmalloc (if jemalloc is not used)" ON)
option(USE_GOOGLE_HEAP_PROFILER "Use Google heap profiler" OFF)
option(USE_GOOGLE_CPU_PROFILER "Use Google cpu profiler" OFF)
