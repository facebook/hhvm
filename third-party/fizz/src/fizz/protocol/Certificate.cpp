/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <fizz/protocol/Certificate.h>

namespace fizz {

IdentityCert::IdentityCert(std::string identity) : identity_(identity) {}

std::string IdentityCert::getIdentity() const {
  return identity_;
}
} // namespace fizz
