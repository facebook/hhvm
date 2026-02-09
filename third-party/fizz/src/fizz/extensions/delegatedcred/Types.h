/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#pragma once

#include <fizz/record/Extensions.h>
#include <fizz/record/Types.h>
#include <folly/Optional.h>

namespace fizz {
namespace extensions {

struct DelegatedCredential {
  uint32_t valid_time;
  SignatureScheme expected_verify_scheme;
  Buf public_key;
  SignatureScheme credential_scheme;
  Buf signature;
  static constexpr ExtensionType extension_type =
      ExtensionType::delegated_credential;
};

struct DelegatedCredentialSupport {
  static constexpr ExtensionType extension_type =
      ExtensionType::delegated_credential;
  std::vector<SignatureScheme> supported_signature_algorithms;
};

enum class DelegatedCredentialMode { Client, Server };

Extension encodeExtension(const extensions::DelegatedCredential& cred);

Extension encodeExtension(const extensions::DelegatedCredentialSupport& cs);
} // namespace extensions

template <>
Status getExtension(
    folly::Optional<extensions::DelegatedCredential>& ret,
    Error& err,
    const std::vector<Extension>& extensions);

template <>
Status getExtension(
    folly::Optional<extensions::DelegatedCredentialSupport>& ret,
    Error& err,
    const std::vector<Extension>& extensions);

} // namespace fizz
