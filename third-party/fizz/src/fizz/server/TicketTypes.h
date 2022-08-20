/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/crypto/Hkdf.h>
#include <fizz/crypto/aead/OpenSSLEVPCipher.h>
#include <fizz/protocol/Types.h>
#include <fizz/server/AeadTicketCipher.h>
#include <fizz/server/TicketCodec.h>

namespace fizz {
namespace server {
using AES128TicketCipher =
    Aead128GCMTicketCipher<TicketCodec<CertificateStorage::X509>>;
using AES128TicketIdentityOnlyCipher =
    Aead128GCMTicketCipher<TicketCodec<CertificateStorage::IdentityOnly>>;
using AES128TokenCipher = Aead128GCMTokenCipher;
} // namespace server
} // namespace fizz
