/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/backend/openssl/OpenSSL.h>
#include <fizz/backend/openssl/certificate/CertUtils.h>
#include <fizz/crypto/signature/Signature.h>

namespace fizz {

using CertUtils = openssl::CertUtils;

namespace detail {
CREATE_FIZZ_FN_ALIAS(getIdentityFromX509, openssl::detail::getIdentityFromX509)
}

} // namespace fizz
