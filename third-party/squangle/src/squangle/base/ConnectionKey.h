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
  virtual ~ConnectionKey() = default;

  [[nodiscard]] virtual size_t hash() const = 0;

  [[nodiscard]] virtual size_t partial_hash() const = 0;

  [[nodiscard]] virtual bool partialEqual(
      const ConnectionKey& rhs) const noexcept = 0;

  [[nodiscard]] virtual std::string getDisplayString(
      bool level2 = false) const = 0;

  [[nodiscard]] virtual const std::string& host() const noexcept = 0;

  [[nodiscard]] virtual const std::string& db_name() const noexcept = 0;

  [[nodiscard]] virtual const std::string& user() const noexcept = 0;

  [[nodiscard]] virtual const std::string& password() const noexcept = 0;

  [[nodiscard]] virtual int port() const noexcept = 0;

  [[nodiscard]] virtual const std::string& special_tag() const noexcept = 0;

  [[nodiscard]] virtual bool operator==(
      const ConnectionKey& rhs) const noexcept = 0;

  [[nodiscard]] bool operator!=(const ConnectionKey& rhs) const noexcept {
    return !(*this == rhs);
  }
};

class MysqlConnectionKey : public ConnectionKey {
 public:
  explicit MysqlConnectionKey(
      folly::StringPiece sp_host = "",
      int sp_port = 0,
      folly::StringPiece sp_db_name = "",
      folly::StringPiece sp_user = "",
      folly::StringPiece sp_password = "",
      folly::StringPiece sp_special_tag = "",
      bool sp_ignore_db_name = false,
      folly::StringPiece sp_unixSocketPath = "");

  [[nodiscard]] bool partialEqual(
      const ConnectionKey& rhs) const noexcept override;

  [[nodiscard]] bool operator==(const MysqlConnectionKey& rhs) const noexcept;

  [[nodiscard]] bool operator!=(const MysqlConnectionKey& rhs) const noexcept {
    return !(*this == rhs);
  }

  [[nodiscard]] virtual bool operator==(
      const ConnectionKey& rhs) const noexcept override;

  [[nodiscard]] const std::string& host() const noexcept override {
    return host_;
  }

  [[nodiscard]] const std::string& db_name() const noexcept override {
    return dbName_;
  }

  [[nodiscard]] const std::string& user() const noexcept override {
    return user_;
  }

  [[nodiscard]] const std::string& password() const noexcept override {
    return password_;
  }

  [[nodiscard]] const std::string& unixSocketPath() const noexcept {
    return unixSocketPath_;
  }

  [[nodiscard]] size_t hash() const noexcept override {
    return hash_;
  }

  [[nodiscard]] size_t partial_hash() const noexcept override {
    return partialHash_;
  }

  [[nodiscard]] int port() const noexcept override {
    return port_;
  }

  [[nodiscard]] const std::string& special_tag() const noexcept override {
    return specialTag_;
  }

  [[nodiscard]] std::string getDisplayString(
      bool level2 = false) const override;

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

template <>
struct hash<facebook::common::mysql_client::MysqlConnectionKey> {
  size_t operator()(
      const facebook::common::mysql_client::MysqlConnectionKey& k) const {
    return k.hash();
  }
};
} // namespace std
