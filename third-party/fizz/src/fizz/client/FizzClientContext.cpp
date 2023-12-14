/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include "fizz/client/FizzClientContext.h"

#include <fizz/protocol/OpenSSLFactory.h>

namespace fizz {
namespace client {

FizzClientContext::FizzClientContext()
    : factory_(std::make_shared<OpenSSLFactory>()),
      clock_(std::make_shared<SystemClock>()) {}

} // namespace client
} // namespace fizz
