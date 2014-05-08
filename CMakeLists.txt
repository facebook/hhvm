CMAKE_MINIMUM_REQUIRED(VERSION 2.8.7 FATAL_ERROR)
PROJECT(hphp C CXX ASM)

IF(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "HHVM requires a 64bit OS")
ENDIF()

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

if (NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third-party/CMakeLists.txt")
  message(FATAL_ERROR "third-party/CMakeLists.txt missing."
                      "Try updating your submodule with:
rm -r third-party
git submodule update --init --recursive
")
endif()

include("${CMAKE_CURRENT_SOURCE_DIR}/CMake/HPHPFunctions.cmake")
include(CheckFunctionExists)

set(HPHP_HOME ${CMAKE_CURRENT_SOURCE_DIR})
set(TP_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third-party")

add_subdirectory(third-party)
add_subdirectory(hphp)

file(GLOB HHVM_CMAKE_FILES "CMake/*.cmake")
install(
  FILES ${HHVM_CMAKE_FILES}
  DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/hphp/CMake"
  COMPONENT dev)
