# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

add_executable(proxygen_pricli PriCli.cpp)
target_compile_options(
    proxygen_pricli PRIVATE
    ${_PROXYGEN_COMMON_COMPILE_OPTIONS}
)
