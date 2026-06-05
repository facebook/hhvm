# Standalone toolchain file for setting common compiler flags for HHVM and dependencies.
option(HPHP_ENABLE_HARDENING "Set hardening flags and definitions, e.g. stack-smashing protection" OFF)
option(HPHP_FORCE_LIBCPP "Force using libc++ as the C++ standard library" OFF)

set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(HPHP_COMPILER_CLANG ON)

include("${CMAKE_CURRENT_LIST_DIR}/HPHPCompiler.cmake")
