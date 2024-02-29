#
# Copyright 2013-2024 Real Logic Limited.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.6.1 FATAL_ERROR)
cmake_policy(VERSION 3.6.1)

file(STRINGS version.txt SBE_VERSION_TXT LIMIT_COUNT 1 REGEX "^[0-9]+(\\.[0-9]+)+")
string(REGEX REPLACE "^([0-9]+(\\.[0-9]+)+).*$" "\\1" SBE_VERSION_NUMERIC "${SBE_VERSION_TXT}")

project("sbe" VERSION "${SBE_VERSION_NUMERIC}")

include(ExternalProject)
include(CheckLibraryExists)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    set(STANDALONE_BUILD TRUE)
endif ()

option(SBE_TESTS "Enable tests" ${STANDALONE_BUILD})
option(SBE_BUILD_SAMPLES "Enable building the sample projects" ${STANDALONE_BUILD})
option(SBE_BUILD_BENCHMARKS "Enable building the benchmarks" ${STANDALONE_BUILD})
option(C_WARNINGS_AS_ERRORS "Enable warnings as errors for C" OFF)
option(CXX_WARNINGS_AS_ERRORS "Enable warnings as errors for C++" OFF)

unset(STANDALONE_BUILD)

if (SBE_TESTS)
    enable_testing()
endif ()

# default built type is Release
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Choose the type of build" FORCE)
endif (NOT CMAKE_BUILD_TYPE)

if (NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
endif ()

if (NOT DEFINED CMAKE_CXX_EXTENSIONS)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif ()

if (NOT DEFINED CMAKE_C_STANDARD)
    set(CMAKE_C_STANDARD 11)
endif ()

if (NOT DEFINED CMAKE_C_EXTENSIONS)
    set(CMAKE_C_EXTENSIONS OFF)
endif ()

if (SBE_TESTS)
    set(SBE_THIRDPARTY_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/thirdparty")

    ExternalProject_Add(
        gtest
        URL ${CMAKE_CURRENT_SOURCE_DIR}/cppbuild/googletest-release-1.12.1.zip
        URL_MD5 2648d4138129812611cf6b6b4b497a3b
        PREFIX "${SBE_THIRDPARTY_BINARY_DIR}/gtest"
        CMAKE_ARGS -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER};-DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        BUILD_BYPRODUCTS "${SBE_THIRDPARTY_BINARY_DIR}/gtest/src/gtest-build/lib/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gmock${CMAKE_STATIC_LIBRARY_SUFFIX};${SBE_THIRDPARTY_BINARY_DIR}/gtest/src/gtest-build/lib/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gmock_main${CMAKE_STATIC_LIBRARY_SUFFIX}"
        INSTALL_COMMAND ""
        LOG_CONFIGURE ON
        LOG_BUILD ON
    )

    ExternalProject_Get_Property(gtest source_dir)
    set(GTEST_SOURCE_DIR ${source_dir})
    ExternalProject_Get_Property(gtest binary_dir)
    set(GTEST_BINARY_DIR ${binary_dir})

    if (MSVC AND CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(GTEST_POSTFIX "d")
    else ()
        set(GTEST_POSTFIX "")
    endif ()

    set(GTEST_LIBS
        ${GTEST_BINARY_DIR}/lib/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gmock${CMAKE_STATIC_LIBRARY_SUFFIX}
        ${GTEST_BINARY_DIR}/lib/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gtest${CMAKE_STATIC_LIBRARY_SUFFIX}
        ${GTEST_BINARY_DIR}/lib/${CMAKE_CFG_INTDIR}/${CMAKE_STATIC_LIBRARY_PREFIX}gtest_main${CMAKE_STATIC_LIBRARY_SUFFIX}
    )
endif ()

find_package(Threads)

if (UNIX)
    add_compile_options(-Wall -Wpedantic -Wextra -Wno-unused-parameter)

    if (C_WARNINGS_AS_ERRORS)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:-Werror>)
    endif (C_WARNINGS_AS_ERRORS)

    if (CXX_WARNINGS_AS_ERRORS)
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-Werror>)
    endif (CXX_WARNINGS_AS_ERRORS)
endif ()

if (APPLE)
    # -Wall doesn't enable everything we want to see
    add_compile_options(-Wsign-compare)
    add_definitions(-DDarwin)
    add_compile_options(-Wno-deprecated-register)
elseif (MSVC)
    add_definitions(-DWIN32)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_NONSTDC_NO_WARNINGS)
    add_definitions(-DNOMINMAX)

    if (${MSVC_VERSION} GREATER_EQUAL 1915)
        # Acknowledge that we understand MSVC resolved a byte alignment issue in this compiler
        add_definitions(-D_ENABLE_EXTENDED_ALIGNED_STORAGE)
    endif ()

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /Od /Zi /MP")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /MP")

    if (C_WARNINGS_AS_ERRORS)
        add_compile_options($<$<COMPILE_LANGUAGE:C>:/WX>)
    endif (C_WARNINGS_AS_ERRORS)

    if (CXX_WARNINGS_AS_ERRORS)
        add_compile_options($<$<COMPILE_LANGUAGE:CXX>:/WX>)
    endif (CXX_WARNINGS_AS_ERRORS)
else ()
    add_definitions(-D_GNU_SOURCE)
endif ()

set(LIBPREFIX "${CMAKE_STATIC_LIBRARY_PREFIX}")
set(LIBSUFFIX "${CMAKE_STATIC_LIBRARY_SUFFIX}")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/binaries")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/lib")

set(SBE_JAR "${CMAKE_CURRENT_SOURCE_DIR}/sbe-all/build/libs/sbe-all-${SBE_VERSION_TXT}.jar")

file(GLOB_RECURSE SBE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/sbe-tool/src/main/java/*.java"
)

add_custom_command(OUTPUT ${SBE_JAR}
    COMMAND ./gradlew assemble -x javadoc --console=plain
    DEPENDS ${SBE_SOURCES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating SBE jar"
)

add_custom_target(sbe-jar
    DEPENDS ${SBE_JAR}
)

set(CODEC_TARGET_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(C_CODEC_TARGET_DIR "${CODEC_TARGET_DIR}/c")
set(CXX_CODEC_TARGET_DIR "${CODEC_TARGET_DIR}/cpp")
set(CODEC_SCHEMA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sbe-tool/src/test/resources")
set(CODEC_PERF_SCHEMA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sbe-benchmarks/src/main/resources")
set(CODEC_EXAMPLES_SCHEMA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/sbe-samples/src/main/resources")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sbe-tool/src/main/cpp)

if (SBE_TESTS)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sbe-tool/src/test/c)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sbe-tool/src/test/cpp)
endif ()

if (SBE_BUILD_SAMPLES)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sbe-samples/src/main/cpp)
endif ()

if (SBE_BUILD_BENCHMARKS)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/sbe-benchmarks/src/main/cpp)
endif ()
