# Standalone toolchain file for setting common compiler flags for HHVM and
# dependencies when building with GCC.
option(HPHP_ENABLE_HARDENING "Set hardening flags and definitions, e.g. stack-smashing protection" OFF)

if(NOT DEFINED CMAKE_C_COMPILER)
  set(CMAKE_C_COMPILER gcc)
endif()
if(NOT DEFINED CMAKE_CXX_COMPILER)
  set(CMAKE_CXX_COMPILER g++)
endif()

set(HPHP_COMPILER_GCC ON)

include("${CMAKE_CURRENT_LIST_DIR}/HPHPCompiler.cmake")
