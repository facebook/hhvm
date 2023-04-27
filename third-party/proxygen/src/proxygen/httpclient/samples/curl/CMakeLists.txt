# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

add_library(proxygencurl CurlClient.cpp)
target_include_directories(
    proxygencurl PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_compile_options(
    proxygencurl PRIVATE
    ${_PROXYGEN_COMMON_COMPILE_OPTIONS}
)

if (BUILD_SHARED_LIBS)
    set_property(TARGET proxygencurl PROPERTY POSITION_INDEPENDENT_CODE ON)
    if (DEFINED PACKAGE_VERSION)
        set_target_properties(
            proxygencurl PROPERTIES VERSION ${PACKAGE_VERSION}
        )
    endif()
endif()

target_link_libraries(proxygencurl PUBLIC proxygen)

install(
    TARGETS proxygencurl
    EXPORT proxygen-exports
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
)

add_executable(proxygen_curl CurlClientMain.cpp)
target_link_libraries(
    proxygen_curl PUBLIC
    proxygencurl
)
target_compile_options(
    proxygen_curl PRIVATE
    ${_PROXYGEN_COMMON_COMPILE_OPTIONS}
)

install(
    TARGETS proxygen_curl
    EXPORT proxygen-exports
    DESTINATION bin
)
