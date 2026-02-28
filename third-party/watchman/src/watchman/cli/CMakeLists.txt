# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

if (IS_FB_BUILD)
    rust_executable(watchmanctl FEATURES fb)
else()
    rust_executable(watchmanctl)
endif()

install_rust_executable(watchmanctl)
