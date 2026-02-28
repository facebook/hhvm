# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

add_library(proxygenwsclientlib WebSocketClient.cpp)
target_include_directories(
    proxygenwsclientlib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)
target_compile_options(
    proxygenwsclientlib PRIVATE
    ${_PROXYGEN_COMMON_COMPILE_OPTIONS}
)

if (BUILD_SHARED_LIBS)
    set_property(
        TARGET proxygenwsclientlib PROPERTY POSITION_INDEPENDENT_CODE ON
    )
    if (DEFINED PACKAGE_VERSION)
        set_target_properties(
            proxygenwsclientlib PROPERTIES VERSION ${PACKAGE_VERSION}
        )
    endif()
endif()

target_link_libraries(proxygenwsclientlib PUBLIC proxygen)

install(
    TARGETS proxygenwsclientlib
    EXPORT proxygen-exports
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
)

add_executable(proxygen_websocket_client main.cpp)
target_link_libraries(
    proxygen_websocket_client PUBLIC
    proxygenwsclientlib
)
target_compile_options(
    proxygen_websocket_client PRIVATE
    ${_PROXYGEN_COMMON_COMPILE_OPTIONS}
)

install(
    TARGETS proxygen_websocket_client
    EXPORT proxygen-exports
    DESTINATION bin
)
