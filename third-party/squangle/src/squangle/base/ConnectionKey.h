/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/String.h>

namespace facebook::common::mysql_client {

// This class encapsulates the data that differentiates 2 connections:
// host, port, db name and user. We also store the password to avoid
// allowing a connection with wrong password be accepted.
// We also store the key as string (without the password and special tag)
// for debugging purposes and to use as keys in other maps
class ConnectionKey {
 public:
  ConnectionKey(
      folly::StringPiece sp_host,
      int sp_port,
      folly::StringPiece sp_db_name,
      folly::StringPiece sp_user,
      folly::StringPiece sp_password,
      folly::StringPiece sp_special_tag = "",
      bool sp_ignore_db_name = false,
      folly::StringPiece sp_unixSocketPath = "");

  FOLLY_NODISCARD bool partialEqual(const ConnectionKey& rhs) const noexcept;

  FOLLY_NODISCARD bool operator==(const ConnectionKey& rhs) const noexcept;

  FOLLY_NODISCARD bool operator!=(const ConnectionKey& rhs) const noexcept {
    return !(*this == rhs);
  }

  FOLLY_NODISCARD const std::string& host() const noexcept {
    return host_;
  }

  FOLLY_NODISCARD const std::string& db_name() const noexcept {
    return dbName_;
  }

  FOLLY_NODISCARD const std::string& user() const noexcept {
    return user_;
  }

  FOLLY_NODISCARD const std::string& password() const noexcept {
    return password_;
  }

  FOLLY_NODISCARD const std::string& unixSocketPath() const noexcept {
    return unixSocketPath_;
  }

  FOLLY_NODISCARD size_t hash() const noexcept {
    return hash_;
  }

  FOLLY_NODISCARD size_t partial_hash() const noexcept {
    return partialHash_;
  }

  FOLLY_NODISCARD int port() const noexcept {
    return port_;
  }

  FOLLY_NODISCARD const std::string& special_tag() const noexcept {
    return specialTag_;
  }

  FOLLY_NODISCARD std::string getDisplayString(bool level2 = false) const;

 private:
  std::string host_;
  std::string dbName_;
  std::string user_;
  std::string password_;
  std::string specialTag_;
  std::string unixSocketPath_;
  size_t partialHash_;
  size_t hash_;
  int port_;
  bool ignoreDbName_;
};

} // namespace facebook::common::mysql_client

// make default template of unordered_map/unordered_set works for ConnectionKey
namespace std {
template <>
struct hash<facebook::common::mysql_client::ConnectionKey> {
  size_t operator()(
      const facebook::common::mysql_client::ConnectionKey& k) const {
    return k.hash();
  }
};
} // namespace std
