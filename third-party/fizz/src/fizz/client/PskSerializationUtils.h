// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include <fizz/client/PskCache.h>
#include <fizz/protocol/Factory.h>

namespace fizz {
namespace client {

std::string serializePsk(const fizz::client::CachedPsk& psk);
fizz::client::CachedPsk deserializePsk(
    const std::string& str,
    const fizz::Factory& factory);

} // namespace client
} // namespace fizz
