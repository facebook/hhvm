/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/exchange/KeyExchange.h>
#include <fizz/protocol/ech/ECHExtensions.h>
#include <fizz/protocol/ech/Types.h>

namespace fizz {
namespace ech {
namespace test {

ParsedECHConfig getParsedECHConfig();
ECHConfig getECHConfig();
ClientHello getClientHelloOuter();
bool isEqual(const ParsedECHConfig& l, const ParsedECHConfig& r);

} // namespace test
} // namespace ech
} // namespace fizz
