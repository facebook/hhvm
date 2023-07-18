/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/Cursor.h>

namespace fizz {
template <>
inline void detail::write<ech::ECHConfig>(
    const ech::ECHConfig& echConfig,
    folly::io::Appender& out) {
  detail::write(echConfig.version, out);
  detail::writeBuf<uint16_t>(echConfig.ech_config_content, out);
}

template <>
inline void detail::write<ech::HpkeSymmetricCipherSuite>(
    const ech::HpkeSymmetricCipherSuite& suite,
    folly::io::Appender& out) {
  detail::write(suite.kdf_id, out);
  detail::write(suite.aead_id, out);
}

template <>
inline void detail::write<ech::HpkeKeyConfig>(
    const ech::HpkeKeyConfig& config,
    folly::io::Appender& out) {
  detail::write(config.config_id, out);
  detail::write(config.kem_id, out);
  detail::writeBuf<uint16_t>(config.public_key, out);
  detail::writeVector<uint16_t>(config.cipher_suites, out);
}

template <>
struct detail::Sizer<ech::ECHConfig> {
  template <class T>
  size_t getSize(const ech::ECHConfig& proto) {
    return sizeof(uint16_t) +
        detail::getBufSize<uint16_t>(proto.ech_config_content);
  }
};

template <>
struct detail::Sizer<ech::HpkeSymmetricCipherSuite> {
  template <class T>
  size_t getSize(const ech::HpkeSymmetricCipherSuite&) {
    return sizeof(uint16_t) + sizeof(uint16_t);
  }
};

template <>
struct detail::Sizer<ech::HpkeKeyConfig> {
  template <class T>
  size_t getSize(const ech::HpkeKeyConfig& config) {
    auto sz = sizeof(uint8_t) + // config_id
        sizeof(uint16_t) + // kem_id
        detail::getBufSize<uint16_t>(config.public_key) +
        sizeof(uint16_t); // len(cipher_suites)
    // Add remaining array length, if any.
    if (!config.cipher_suites.empty()) {
      sz += config.cipher_suites.size() *
          detail::getSize(config.cipher_suites.front());
    }
    return sz;
  }
};

template <>
struct detail::Reader<ech::HpkeSymmetricCipherSuite> {
  template <class T>
  size_t read(ech::HpkeSymmetricCipherSuite& suite, folly::io::Cursor& cursor) {
    size_t len = detail::read(suite.kdf_id, cursor) +
        detail::read(suite.aead_id, cursor);
    return len;
  }
};

template <>
struct detail::Reader<ech::ECHConfig> {
  template <class T>
  size_t read(ech::ECHConfig& echConfig, folly::io::Cursor& cursor) {
    size_t len = 0;
    len += detail::read(echConfig.version, cursor);
    len += readBuf<uint16_t>(echConfig.ech_config_content, cursor);
    return len;
  }
};

template <>
struct detail::Reader<ech::HpkeKeyConfig> {
  template <class T>
  size_t read(ech::HpkeKeyConfig& keyConfig, folly::io::Cursor& cursor) {
    size_t len = 0;
    len += detail::read(keyConfig.config_id, cursor);
    len += detail::read(keyConfig.kem_id, cursor);
    len += readBuf<uint16_t>(keyConfig.public_key, cursor);
    len += readVector<uint16_t>(keyConfig.cipher_suites, cursor);
    return len;
  }
};

template <>
struct detail::Reader<std::array<uint8_t, 16>> {
  template <class T>
  size_t read(std::array<uint8_t, 16>& out, folly::io::Cursor& cursor) {
    cursor.pull(out.data(), out.size());
    return out.size();
  }
};

template <>
inline Buf encode<ech::ECHConfigContentDraft>(
    ech::ECHConfigContentDraft&& ech) {
  size_t extLen = [&]() {
    size_t sz = 0;
    for (const auto& ext : ech.extensions) {
      sz += detail::getSize(ext);
    }
    return sizeof(uint16_t) + sz;
  }();
  auto buf = folly::IOBuf::create(
      detail::getSize(ech.key_config) + sizeof(uint8_t) + // maximum_name_length
      detail::getBufSize<uint16_t>(ech.public_name) + extLen); // extensions

  folly::io::Appender appender(buf.get(), 0);
  detail::write(ech.key_config, appender);
  detail::write(ech.maximum_name_length, appender);
  detail::writeBuf<uint8_t>(ech.public_name, appender);
  detail::writeVector<uint16_t>(ech.extensions, appender);
  return buf;
}

template <>
inline Buf encode<const ech::ECHConfig&>(const ech::ECHConfig& echConfig) {
  auto buf = folly::IOBuf::create(
      sizeof(uint16_t) +
      detail::getBufSize<uint16_t>(echConfig.ech_config_content));

  folly::io::Appender appender(buf.get(), 20);
  detail::write(echConfig.version, appender);
  detail::writeBuf<uint16_t>(echConfig.ech_config_content, appender);

  return buf;
}

template <>
inline Buf encode<ech::ECHConfig>(ech::ECHConfig&& echConfig) {
  return encode<const ech::ECHConfig&>(echConfig);
}

template <>
inline ech::ECHConfigContentDraft decode(folly::io::Cursor& cursor) {
  ech::ECHConfigContentDraft echConfigContent;
  detail::read(echConfigContent.key_config, cursor);
  detail::read(echConfigContent.maximum_name_length, cursor);
  detail::readBuf<uint8_t>(echConfigContent.public_name, cursor);
  detail::readVector<uint16_t>(echConfigContent.extensions, cursor);

  return echConfigContent;
}

template <>
inline ech::ECHConfig decode(folly::io::Cursor& cursor) {
  ech::ECHConfig echConfig;
  detail::read(echConfig.version, cursor);
  detail::readBuf<uint16_t>(echConfig.ech_config_content, cursor);

  return echConfig;
}

template <>
inline ech::ECHConfigList decode(folly::io::Cursor& cursor) {
  ech::ECHConfigList echConfigList;
  detail::readVector<uint16_t>(echConfigList.configs, cursor);
  return echConfigList;
}

template <>
inline Buf encode<const ech::ECHConfigList&>(
    const ech::ECHConfigList& echConfigList) {
  auto buf = folly::IOBuf::create(sizeof(uint16_t));
  folly::io::Appender appender(buf.get(), 20);
  detail::writeVector<uint16_t>(echConfigList.configs, appender);
  return buf;
}

template <>
inline Buf encode<ech::ECHConfigList>(ech::ECHConfigList&& echConfigList) {
  return encode<const ech::ECHConfigList&>(echConfigList);
}

} // namespace fizz
