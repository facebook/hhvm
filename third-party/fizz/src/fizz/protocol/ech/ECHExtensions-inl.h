/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/record/Types.h>
#include <vector>

namespace fizz {
template <>
inline Extension encodeExtension(const ech::ClientECH& clientECH) {
  Extension ext;
  ext.extension_type = ExtensionType::encrypted_client_hello;
  ext.extension_data = folly::IOBuf::create(0);

  folly::io::Appender appender(ext.extension_data.get(), 20);
  detail::write(clientECH.cipher_suite, appender);
  detail::writeBuf<uint8_t>(clientECH.config_id, appender);
  detail::writeBuf<uint16_t>(clientECH.enc, appender);
  detail::writeBuf<uint16_t>(clientECH.payload, appender);

  return ext;
}

template <>
inline Extension encodeExtension(const ech::ServerECH& serverECH) {
  Extension ext;
  ext.extension_type = ExtensionType::encrypted_client_hello;
  ext.extension_data = folly::IOBuf::create(0);

  folly::io::Appender appender(ext.extension_data.get(), 20);

  detail::writeVector<uint16_t>(serverECH.retry_configs, appender);
  return ext;
}

template <>
inline Extension encodeExtension(const ech::ECHIsInner&) {
  Extension ext;
  ext.extension_type = ExtensionType::ech_is_inner;
  ext.extension_data = folly::IOBuf::create(0);
  return ext;
}

template <>
inline Extension encodeExtension(const ech::OuterExtensions& outerExts) {
  Extension ext;
  ext.extension_type = ExtensionType::ech_outer_extensions;
  ext.extension_data = folly::IOBuf::create(0);

  folly::io::Appender appender(ext.extension_data.get(), 20);
  detail::writeVector<uint8_t>(outerExts.types, appender);

  return ext;
}

template <>
inline ech::ClientECH getExtension(folly::io::Cursor& cs) {
  ech::ClientECH clientECH;
  detail::read(clientECH.cipher_suite, cs);
  detail::readBuf<uint8_t>(clientECH.config_id, cs);
  detail::readBuf<uint16_t>(clientECH.enc, cs);
  detail::readBuf<uint16_t>(clientECH.payload, cs);

  return clientECH;
}

template <>
inline ech::ServerECH getExtension(folly::io::Cursor& cs) {
  ech::ServerECH serverECH;
  detail::readVector<uint16_t>(serverECH.retry_configs, cs);

  return serverECH;
}

template <>
inline ech::ECHIsInner getExtension(folly::io::Cursor&) {
  return ech::ECHIsInner();
}

template <>
inline ech::OuterExtensions getExtension(folly::io::Cursor& cs) {
  ech::OuterExtensions outerExts;
  detail::readVector<uint8_t>(outerExts.types, cs);
  return outerExts;
}
} // namespace fizz
