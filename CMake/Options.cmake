#set(CMAKE_BUILD_TYPE Debug)

option(ALWAYS_ASSERT "Enabled asserts in a release build" OFF)
option(ENABLE_SSP "Enabled GCC/LLVM stack-smashing protection" OFF)
option(STATIC_CXX_LIB "Statically link libstd++ and libgcc." OFF)
option(ENABLE_AVX2 "Enable the use of AVX2 instructions" OFF)
option(ENABLE_AARCH64_CRC "Enable the use of CRC instructions" OFF)
option(ENABLE_FASTCGI "Enable the FastCGI interface." ON)
option(ENABLE_SSE4_2 "Enable SSE4.2 supported code." OFF)

option(EXECUTION_PROFILER "Enable the execution profiler" OFF)

option(USE_JEMALLOC "Use jemalloc" ON)
option(FORCE_TP_JEMALLOC "Always build and statically link jemalloc instead of using system version" OFF)

option(ENABLE_HHPROF "Enable HHProf" OFF)

option(CLANG_FORCE_LIBSTDCXX "Force libstdc++ when building against Clang/LLVM" OFF)

option(USE_TCMALLOC "Use tcmalloc (if jemalloc is not used)" ON)
option(USE_GOOGLE_HEAP_PROFILER "Use Google heap profiler" OFF)
option(USE_GOOGLE_CPU_PROFILER "Use Google cpu profiler" OFF)

option(DISABLE_HARDWARE_COUNTERS "Disable hardware counters (for XenU systems)" OFF)

option(ENABLE_TRACE "Enable tracing in release build" OFF)
option(CPACK_GENERATOR "Enable build of distribution packages using CPack" OFF)

option(ENABLE_COTIRE "Speed up the build by precompiling headers" OFF)
if (APPLE)
  option(ENABLE_MCROUTER "Build the mcrouter library and extension" OFF)
else()
  option(ENABLE_MCROUTER "Build the mcrouter library and extension" ON)
endif()
option(ENABLE_PROXYGEN_SERVER "Build the Proxygen HTTP server" ON)

option(ENABLE_SPLIT_DWARF "Reduce linker memory usage by putting debugging information into .dwo files" OFF)

IF (LINUX)
    option(MAP_TEXT_HUGE_PAGES "Remap hot static code onto huge pages" ON)
ENDIF()

IF (NOT DEFAULT_CONFIG_DIR)
  set(DEFAULT_CONFIG_DIR "/etc/hhvm/" CACHE STRING
    "Default directory to find php.ini")
ENDIF()

option(ENABLE_XED "Use the XED library for HHVM. If ON, tc-print will be built for X86." OFF)
