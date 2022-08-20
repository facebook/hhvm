/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/String.h>

namespace facebook {
namespace common {
namespace mysql_client {

// This class encapsulates the data that differentiates 2 connections:
// host, port, db name and user. We also store the password to avoid
// allowing a connection with wrong password be accepted.
// We also store the key as string (without the password and special tag)
// for debugging purposes and to use as keys in other maps
class ConnectionKey {
 public:
  const std::string host;
  const int port;
  const std::string db_name;
  const std::string user;
  // keeping password to avoid password error
  const std::string password;
  const std::string special_tag;
  const bool ignore_db_name;
  const size_t partial_hash;
  const size_t hash;

  ConnectionKey(
      folly::StringPiece sp_host,
      int sp_port,
      folly::StringPiece sp_db_name,
      folly::StringPiece sp_user,
      folly::StringPiece sp_password,
      folly::StringPiece sp_special_tag = "",
      bool sp_ignore_db_name = false);

  bool partialEqual(const ConnectionKey& rhs) const;

  bool operator==(const ConnectionKey& rhs) const;

  bool operator!=(const ConnectionKey& rhs) const {
    return !(*this == rhs);
  }

  std::string getDisplayString(bool level2 = false) const;
};
} // namespace mysql_client
} // namespace common
} // namespace facebook

// make default template of unordered_map/unordered_set works for ConnectionKey
namespace std {
template <>
struct hash<facebook::common::mysql_client::ConnectionKey> {
  size_t operator()(
      const facebook::common::mysql_client::ConnectionKey& k) const {
    return k.hash;
  }
};
} // namespace std
