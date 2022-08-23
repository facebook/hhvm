/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/base/ConnectionKey.h"

#include <folly/Format.h>
#include <folly/hash/Hash.h>

namespace facebook {
namespace common {
namespace mysql_client {

ConnectionKey::ConnectionKey(
    folly::StringPiece sp_host,
    int sp_port,
    folly::StringPiece sp_db_name,
    folly::StringPiece sp_user,
    folly::StringPiece sp_password,
    folly::StringPiece sp_special_tag,
    bool sp_ignore_db_name,
    folly::StringPiece sp_unixSocketPath)
    : host(sp_host.toString()),
      port(sp_port),
      db_name(sp_db_name.toString()),
      user(sp_user.toString()),
      password(sp_password.toString()),
      special_tag(sp_special_tag.toString()),
      ignore_db_name(sp_ignore_db_name),
      unixSocketPath(sp_unixSocketPath),
      partial_hash(folly::Hash()(
          sp_host,
          sp_port,
          sp_user,
          sp_password,
          sp_special_tag)),
      hash(
          ignore_db_name ? partial_hash
                         : folly::Hash()(partial_hash, sp_db_name)) {}

bool ConnectionKey::operator==(const ConnectionKey& rhs) const {
  return hash == rhs.hash && host == rhs.host && port == rhs.port &&
      (ignore_db_name || db_name == rhs.db_name) && user == rhs.user &&
      password == rhs.password && special_tag == rhs.special_tag &&
      unixSocketPath == rhs.unixSocketPath;
}

bool ConnectionKey::partialEqual(const ConnectionKey& rhs) const {
  return partial_hash == rhs.partial_hash && host == rhs.host &&
      port == rhs.port && user == rhs.user && password == rhs.password &&
      special_tag == rhs.special_tag && unixSocketPath == rhs.unixSocketPath;
}

std::string ConnectionKey::getDisplayString(bool level2) const {
  if (unixSocketPath.empty()) {
    return folly::format(
               "{} [{}] ({}@{}:{})",
               level2 ? "" : db_name,
               special_tag,
               user,
               host,
               port)
        .str();
  }

  return folly::format(
             "{} [{}] ({}@{})",
             level2 ? "" : db_name,
             special_tag,
             user,
             unixSocketPath)
      .str();
}
} // namespace mysql_client
} // namespace common
} // namespace facebook
