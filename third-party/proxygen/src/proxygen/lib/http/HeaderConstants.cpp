/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/HeaderConstants.h>

namespace proxygen { namespace headers {

const std::string kAuthority(":authority");
const std::string kMethod(":method");
const std::string kPath(":path");
const std::string kScheme(":scheme");
const std::string kStatus(":status");
const std::string kProtocol(":protocol");

const std::string kHttp("http");
const std::string kHttps("https");
const std::string kMasque("masque");

const std::string kWebsocketString("websocket");
const std::string kStatus200("200");

}} // namespace proxygen::headers
