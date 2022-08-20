# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

if (NOT DEFINED LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR "lib")
endif()

add_subdirectory(external)
add_subdirectory(lib)
add_subdirectory(httpserver)
add_subdirectory(httpclient)
add_subdirectory(fuzzers)
