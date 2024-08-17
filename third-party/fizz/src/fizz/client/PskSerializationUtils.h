// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include <fizz/client/PskCache.h>
#include <fizz/protocol/Certificate.h>

namespace fizz {
namespace client {
std::string serializePsk(
    const CertificateSerialization& serializer,
    const fizz::client::CachedPsk& psk);
fizz::client::CachedPsk deserializePsk(
    const CertificateSerialization& serializer,
    const std::string& str);
} // namespace client
} // namespace fizz
