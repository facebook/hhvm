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
inline Extension encodeExtension(const ech::OuterClientECH& clientECH) {
  Extension ext;
  ext.extension_type = ExtensionType::encrypted_client_hello;
  ext.extension_data = folly::IOBuf::create(0);

  folly::io::Appender appender(ext.extension_data.get(), 20);
  detail::write(clientECH.ech_type, appender);
  detail::write(clientECH.cipher_suite, appender);
  detail::write(clientECH.config_id, appender);
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
inline Extension encodeExtension(const ech::InnerClientECH& innerECH) {
  Extension ext;
  ext.extension_type = ExtensionType::encrypted_client_hello;
  ext.extension_data = folly::IOBuf::create(0);

  folly::io::Appender appender(ext.extension_data.get(), 4);
  detail::write(innerECH.ech_type, appender);
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
inline Extension encodeExtension(const ech::ECHAcceptanceConfirmation& conf) {
  Extension ext;
  ext.extension_type = ExtensionType::encrypted_client_hello;
  ext.extension_data = folly::IOBuf::create(8);

  folly::io::Appender appender(ext.extension_data.get(), 0);
  appender.push(conf.payload.data(), conf.payload.size());

  return ext;
}

template <>
inline ech::OuterClientECH getExtension(folly::io::Cursor& cs) {
  ech::ClientECHType type;
  detail::read(type, cs);
  if (type != ech::ClientECHType::Outer) {
    throw std::runtime_error("wrong ech variant");
  }
  ech::OuterClientECH clientECH;
  detail::read(clientECH.cipher_suite, cs);
  detail::read(clientECH.config_id, cs);
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
inline ech::InnerClientECH getExtension(folly::io::Cursor& cs) {
  ech::ClientECHType type;
  detail::read(type, cs);
  if (type != ech::ClientECHType::Inner) {
    throw std::runtime_error("wrong ech variant");
  }
  return ech::InnerClientECH();
}

template <>
inline ech::OuterExtensions getExtension(folly::io::Cursor& cs) {
  ech::OuterExtensions outerExts;
  detail::readVector<uint8_t>(outerExts.types, cs);
  return outerExts;
}

template <>
inline ech::ECHAcceptanceConfirmation getExtension(folly::io::Cursor& cs) {
  ech::ECHAcceptanceConfirmation conf;
  cs.pull(conf.payload.data(), conf.payload.size());
  return conf;
}
} // namespace fizz
