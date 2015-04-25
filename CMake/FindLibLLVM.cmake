# Find LLVM libraries and headers
# find_package when used in CONFIG mode will look for LLVMConfig.cmake and it will set variables such as  
#
#LLVM_CMAKE_DIR
#    The path to the LLVM CMake directory (i.e. the directory containing LLVMConfig.cmake).
#LLVM_DEFINITIONS
#    A list of preprocessor defines that should be used when building against LLVM.
#LLVM_ENABLE_ASSERTIONS
#    This is set to ON if LLVM was built with assertions, otherwise OFF.
#LLVM_ENABLE_EH
#    This is set to ON if LLVM was built with exception handling (EH) enabled, otherwise OFF.
#LLVM_ENABLE_RTTI
#    This is set to ON if LLVM was built with run time type information (RTTI), otherwise OFF.
#LLVM_INCLUDE_DIRS
#    A list of include paths to directories containing LLVM header files.
#LLVM_PACKAGE_VERSION
#LLVM_TOOLS_BINARY_DIR
#    The path to the directory containing the LLVM tools (e.g. llvm-as). 

cmake_minimum_required(VERSION 2.8.8)
find_package(LLVM REQUIRED CONFIG)

IF (LLVM_DIR)
  message(STATUS "Found LLVM version: ${LLVM_PACKAGE_VERSION}")
  message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
  message(STATUS "Using these default LLVM definitions: ${LLVM_DEFINITIONS}")

  llvm_map_components_to_libnames(LIBLLVM_LIBRARY all)
  MARK_AS_ADVANCED(LIBLLVM_LIBRARY LLVM_INCLUDE_DIRS LLVM_DEFINITIONS)

ELSE()
  MESSAGE(STATUS "Could not find LLVMConfig.cmake .The location can be specified via passing -DLLVM_DIR=<PREFIX>/share/llvm/cmake")
ENDIF()
