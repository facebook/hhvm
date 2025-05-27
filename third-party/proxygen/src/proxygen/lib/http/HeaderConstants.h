/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

namespace proxygen::headers {

#ifndef CLANG_LAZY_INIT_TEST
#define CLANG_LAZY_INIT_TEST
#endif

CLANG_LAZY_INIT_TEST extern const std::string kAuthority;
CLANG_LAZY_INIT_TEST extern const std::string kMethod;
CLANG_LAZY_INIT_TEST extern const std::string kPath;
CLANG_LAZY_INIT_TEST extern const std::string kScheme;
CLANG_LAZY_INIT_TEST extern const std::string kStatus;
CLANG_LAZY_INIT_TEST extern const std::string kProtocol;

CLANG_LAZY_INIT_TEST extern const std::string kHttp;
CLANG_LAZY_INIT_TEST extern const std::string kHttps;
CLANG_LAZY_INIT_TEST extern const std::string kMasque;

CLANG_LAZY_INIT_TEST extern const std::string kWebsocketString;
CLANG_LAZY_INIT_TEST extern const std::string kStatus200;

} // namespace proxygen::headers
