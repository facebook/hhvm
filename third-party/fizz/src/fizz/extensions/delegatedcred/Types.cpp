/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#include <fizz/extensions/delegatedcred/Types.h>
#include <fizz/record/Types.h>

#include <folly/io/Cursor.h>

using namespace fizz::extensions;

namespace fizz {

template <>
Status getExtension(
    folly::Optional<DelegatedCredential>& ret,
    Error& err,
    const std::vector<Extension>& extensions) {
  auto it = findExtension(extensions, ExtensionType::delegated_credential);
  if (it == extensions.end()) {
    ret = folly::none;
    return Status::Success;
  }
  DelegatedCredential cred;
  folly::io::Cursor cursor(it->extension_data.get());
  size_t len;
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, cred.valid_time, cursor));
  FIZZ_RETURN_ON_ERROR(
      detail::read(len, err, cred.expected_verify_scheme, cursor));
  detail::readBuf<detail::bits24>(cred.public_key, cursor);
  FIZZ_RETURN_ON_ERROR(detail::read(len, err, cred.credential_scheme, cursor));
  detail::readBuf<uint16_t>(cred.signature, cursor);
  ret = std::move(cred);
  return Status::Success;
}

template <>
Status getExtension(
    folly::Optional<DelegatedCredentialSupport>& ret,
    Error& err,
    const std::vector<Extension>& extensions) {
  auto it = findExtension(extensions, ExtensionType::delegated_credential);
  if (it == extensions.end()) {
    ret = folly::none;
    return Status::Success;
  }
  DelegatedCredentialSupport supp;
  folly::io::Cursor cursor(it->extension_data.get());
  size_t len;
  FIZZ_RETURN_ON_ERROR(
      detail::readVector<uint16_t>(
          len, err, supp.supported_signature_algorithms, cursor));
  ret = supp;
  return Status::Success;
}

namespace extensions {
Extension encodeExtension(const DelegatedCredential& cred) {
  Extension ext;
  ext.extension_type = ExtensionType::delegated_credential;
  ext.extension_data = folly::IOBuf::create(10);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  Error err;
  FIZZ_THROW_ON_ERROR(detail::write(err, cred.valid_time, appender), err);
  FIZZ_THROW_ON_ERROR(
      detail::write(err, cred.expected_verify_scheme, appender), err);
  FIZZ_THROW_ON_ERROR(
      detail::writeBuf<detail::bits24>(err, cred.public_key, appender), err);
  FIZZ_THROW_ON_ERROR(
      detail::write(err, cred.credential_scheme, appender), err);
  FIZZ_THROW_ON_ERROR(
      detail::writeBuf<uint16_t>(err, cred.signature, appender), err);
  return ext;
}

Extension encodeExtension(const DelegatedCredentialSupport& supp) {
  Extension ext;
  ext.extension_type = ExtensionType::delegated_credential;
  ext.extension_data = folly::IOBuf::create(10);
  folly::io::Appender appender(ext.extension_data.get(), 10);
  Error err;
  FIZZ_THROW_ON_ERROR(
      detail::writeVector<uint16_t>(
          err, supp.supported_signature_algorithms, appender),
      err);
  return ext;
}
} // namespace extensions

} // namespace fizz
