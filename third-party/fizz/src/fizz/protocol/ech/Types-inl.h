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
inline void detail::write<ech::ECHConfigContentDraft>(
    const ech::ECHConfigContentDraft& echConfigContent,
    folly::io::Appender& out) {
  detail::write(echConfigContent.key_config, out);
  detail::write(echConfigContent.maximum_name_length, out);
  detail::writeString<uint8_t>(echConfigContent.public_name, out);
  detail::writeVector<uint16_t>(echConfigContent.extensions, out);
}

template <>
inline void detail::write<ech::ECHConfig>(
    const ech::ECHConfig& echConfig,
    folly::io::Appender& out) {
  detail::write(echConfig.version, out);
  detail::writeBuf<uint16_t>(echConfig.ech_config_content, out);
}

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
struct detail::Sizer<ech::ECHConfigContentDraft> {
  template <class T>
  size_t getSize(const ech::ECHConfigContentDraft& echConfigContent) {
    auto sz = detail::getSize(echConfigContent.key_config) +
        sizeof(uint8_t) + // maximum_name_length
        detail::getStringSize<uint8_t>(echConfigContent.public_name) +
        sizeof(uint16_t); // len(extensions)
    // extensions
    for (const auto& ext : echConfigContent.extensions) {
      sz += detail::getSize(ext);
    }
    return sz;
  }
};

template <>
struct detail::Sizer<ech::ECHConfig> {
  template <class T>
  size_t getSize(const ech::ECHConfig& echConfig) {
    return sizeof(uint16_t) + // version
        detail::getBufSize<uint16_t>(echConfig.ech_config_content);
  }
};

// template <>
// struct detail::Reader<std::array<uint8_t, 16>> {
//   template <class T>
//   size_t read(std::array<uint8_t, 16>& out, folly::io::Cursor& cursor) {
//     cursor.pull(out.data(), out.size());
//     return out.size();
//   }
// };

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
struct detail::Reader<ech::HpkeKeyConfig> {
  template <class T>
  size_t read(ech::HpkeKeyConfig& keyConfig, folly::io::Cursor& cursor) {
    size_t len = 0;
    len += detail::read(keyConfig.config_id, cursor);
    len += detail::read(keyConfig.kem_id, cursor);
    len += readBuf<uint16_t>(keyConfig.public_key, cursor);
    if (!keyConfig.public_key) {
      throw std::runtime_error("Invalid public key length");
    }
    len += readVector<uint16_t>(keyConfig.cipher_suites, cursor);
    return len;
  }
};

template <>
struct detail::Reader<ech::ECHConfigContentDraft> {
  template <class T>
  size_t read(
      ech::ECHConfigContentDraft& echConfigContent,
      folly::io::Cursor& cursor) {
    size_t len = 0;
    len += detail::read(echConfigContent.key_config, cursor);
    len += detail::read(echConfigContent.maximum_name_length, cursor);
    len += detail::readString<uint8_t>(echConfigContent.public_name, cursor);
    if (echConfigContent.public_name.empty()) {
      throw std::runtime_error("Invalid public name length");
    }
    len += detail::readVector<uint16_t>(echConfigContent.extensions, cursor);
    return len;
  }
};

template <>
struct detail::Reader<ech::ECHConfig> {
  template <class T>
  size_t read(ech::ECHConfig& echConfig, folly::io::Cursor& cursor) {
    size_t len = 0;
    len += detail::read(echConfig.version, cursor);
    len += detail::readBuf<uint16_t>(echConfig.ech_config_content, cursor);
    return len;
  }
};

template <>
inline Buf encode<const ech::ECHConfigContentDraft&>(
    const ech::ECHConfigContentDraft& echConfigContent) {
  auto buf = folly::IOBuf::create(detail::getSize(echConfigContent));
  folly::io::Appender appender(buf.get(), 20);
  detail::write(echConfigContent, appender);
  return buf;
}

template <>
inline Buf encode<ech::ECHConfigContentDraft>(
    ech::ECHConfigContentDraft&& echConfigContent) {
  return encode<const ech::ECHConfigContentDraft&>(echConfigContent);
}

template <>
inline Buf encode<const ech::ECHConfig&>(const ech::ECHConfig& echConfig) {
  auto buf = folly::IOBuf::create(detail::getSize(echConfig));
  folly::io::Appender appender(buf.get(), 20);
  detail::write(echConfig, appender);
  return buf;
}

template <>
inline Buf encode<ech::ECHConfig>(ech::ECHConfig&& echConfig) {
  return encode<const ech::ECHConfig&>(echConfig);
}

template <>
inline ech::ECHConfigContentDraft decode(folly::io::Cursor& cursor) {
  ech::ECHConfigContentDraft echConfigContent;
  detail::read(echConfigContent, cursor);
  return echConfigContent;
}

template <>
inline ech::ECHConfig decode(folly::io::Cursor& cursor) {
  ech::ECHConfig echConfig;
  detail::read(echConfig, cursor);
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
  size_t len = sizeof(uint16_t);
  for (const auto& config : echConfigList.configs) {
    len += detail::getSize(config);
  }

  auto buf = folly::IOBuf::create(len);
  folly::io::Appender appender(buf.get(), 20);
  detail::writeVector<uint16_t>(echConfigList.configs, appender);
  return buf;
}

template <>
inline Buf encode<ech::ECHConfigList>(ech::ECHConfigList&& echConfigList) {
  return encode<const ech::ECHConfigList&>(echConfigList);
}
} // namespace fizz
