/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

namespace proxygen { namespace headers {
extern const std::string kAuthority;
extern const std::string kMethod;
extern const std::string kPath;
extern const std::string kScheme;
extern const std::string kStatus;
extern const std::string kProtocol;

extern const std::string kHttp;
extern const std::string kHttps;
extern const std::string kMasque;

extern const std::string kWebsocketString;
extern const std::string kStatus200;

}} // namespace proxygen::headers
