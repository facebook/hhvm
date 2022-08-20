/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <folly/String.h>
#include <folly/hash/Hash.h>
#include <string>

namespace wangle {

enum class CertCrypto { BEST_AVAILABLE, SHA1_SIGNATURE };

struct dn_char_traits : public std::char_traits<char> {
  static bool eq(char c1, char c2) {
    return ::tolower(c1) == ::tolower(c2);
  }

  static bool ne(char c1, char c2) {
    return ::tolower(c1) != ::tolower(c2);
  }

  static bool lt(char c1, char c2) {
    return ::tolower(c1) < ::tolower(c2);
  }

  static int compare(const char* s1, const char* s2, size_t n) {
    while (n--) {
      if (::tolower(*s1) < ::tolower(*s2)) {
        return -1;
      }
      if (::tolower(*s1) > ::tolower(*s2)) {
        return 1;
      }
      ++s1;
      ++s2;
    }
    return 0;
  }

  static const char* find(const char* s, size_t n, char a) {
    char la = ::tolower(a);
    while (n--) {
      if (::tolower(*s) == la) {
        return s;
      } else {
        ++s;
      }
    }
    return nullptr;
  }
};

// Case insensitive string
typedef std::basic_string<char, dn_char_traits> DNString;

struct SSLContextKey {
  DNString dnString;
  CertCrypto certCrypto;

  explicit SSLContextKey(
      DNString dns,
      CertCrypto crypto = CertCrypto::BEST_AVAILABLE)
      : dnString(dns), certCrypto(crypto) {}

  bool operator==(const SSLContextKey& rhs) const {
    return dnString == rhs.dnString && certCrypto == rhs.certCrypto;
  }
};

struct SSLContextKeyHash {
  size_t operator()(const SSLContextKey& sslContextKey) const noexcept {
    std::string lowercase(
        sslContextKey.dnString.data(), sslContextKey.dnString.size());
    folly::toLowerAscii(lowercase);
    return folly::hash::hash_combine(
        lowercase, static_cast<int>(sslContextKey.certCrypto));
  }
};

} // namespace wangle
