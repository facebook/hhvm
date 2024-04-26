/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/certificate/OpenSSLPeerCertImpl.h>
#include <fizz/crypto/signature/Signature.h>
#include <fizz/protocol/CertUtils.h>

namespace fizz {

template <KeyType T>
using OpenSSLPeerCertImpl = openssl::OpenSSLPeerCertImpl<T>;

} // namespace fizz
