/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/fizz-config.h>

#if FIZZ_CERTIFICATE_USE_OPENSSL_CERT

#include <fizz/backend/openssl/certificate/OpenSSLCertificateVerifier.h>

namespace fizz {
using DefaultCertificateVerifier = ::fizz::openssl::OpenSSLCertificateVerifier;
} // namespace fizz

#else // !FIZZ_CERTIFICATE_USE_OPENSSL_CERT

#include <fizz/protocol/CertificateVerifier.h>

namespace fizz {
using DefaultCertificateVerifier = ::fizz::TerminatingCertificateVerifier;
} // namespace fizz
#endif // FIZZ_CERTIFICATE_USE_OPENSSL_CERT
