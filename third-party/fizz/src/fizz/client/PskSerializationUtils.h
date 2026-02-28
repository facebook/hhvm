// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include <fizz/client/PskCache.h>
#include <fizz/protocol/Certificate.h>

namespace fizz {
namespace client {
Status serializePsk(
    std::string& ret,
    Error& err,
    const CertificateSerialization& serializer,
    const fizz::client::CachedPsk& psk);
Status deserializePsk(
    fizz::client::CachedPsk& ret,
    Error& err,
    const CertificateSerialization& serializer,
    folly::ByteRange psk);
} // namespace client
} // namespace fizz
