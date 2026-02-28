/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <map>

#include <fizz/protocol/Certificate.h>

namespace fizz {
enum class MatchType { Direct, Default };
struct CertMatchStruct {
  std::shared_ptr<SelfCert> cert;
  SignatureScheme scheme;
  MatchType type;
};
using CertMatch = folly::Optional<CertMatchStruct>;
} // namespace fizz
