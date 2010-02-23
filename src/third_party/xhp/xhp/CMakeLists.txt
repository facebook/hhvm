cmake_minimum_required(VERSION 2.6)
project(libxhp CXX)

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})
SET(CMAKE_CXX_FLAGS "-fPIC -g -Wall -O3 -minline-all-stringops")

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
find_package(RE2C REQUIRED)

if (${RE2C_VERSION} VERSION_LESS 0.13.0)
	message(FATAL_ERROR "re2c is too old, found ${RE2C_VERSION} and we need 0.13.0")
endif()

FLEX_TARGET(XHPScanner ${CMAKE_CURRENT_SOURCE_DIR}/scanner.l ${CMAKE_CURRENT_SOURCE_DIR}/scanner.lex.cpp COMPILE_FLAGS " -C --header-file=scanner.lex.hpp")
BISON_TARGET(XHPParser ${CMAKE_CURRENT_SOURCE_DIR}/parser.y ${CMAKE_CURRENT_SOURCE_DIR}/parser.yacc.cpp  COMPILE_FLAGS " --debug --verbose -d")
ADD_FLEX_BISON_DEPENDENCY(XHPScanner XHPParser)

RE2C_TARGET(${CMAKE_CURRENT_SOURCE_DIR}/fastpath.re ${CMAKE_CURRENT_SOURCE_DIR}/fastpath.cpp "-c")

add_library(xhp STATIC code_rope.cpp xhp_preprocess.cpp xhpize.cpp fastpath.cpp ${FLEX_XHPScanner_OUTPUTS} ${BISON_XHPParser_OUTPUTS})

INSTALL(TARGETS xhp DESTINATION lib)