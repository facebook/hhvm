/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "squangle/base/ConnectionKey.h"

#include <boost/polymorphic_cast.hpp>
#include <folly/hash/Hash.h>

namespace facebook {
namespace common {
namespace mysql_client {

MysqlConnectionKey::MysqlConnectionKey(
    folly::StringPiece host,
    int port,
    folly::StringPiece db_name,
    folly::StringPiece user,
    folly::StringPiece password,
    folly::StringPiece special_tag,
    bool ignore_db_name,
    folly::StringPiece unixSocketPath)
    : host_(host.toString()),
      dbName_(db_name.toString()),
      user_(user.toString()),
      password_(password.toString()),
      specialTag_(special_tag.toString()),
      unixSocketPath_(unixSocketPath),
      partialHash_(folly::Hash()(host, port, user, password, special_tag)),
      hash_(
          ignore_db_name ? partialHash_ : folly::Hash()(partialHash_, dbName_)),
      port_(port),
      ignoreDbName_(ignore_db_name) {}

bool MysqlConnectionKey::operator==(
    const MysqlConnectionKey& rhs) const noexcept {
  return hash_ == rhs.hash_ && host_ == rhs.host_ && port_ == rhs.port_ &&
      (ignoreDbName_ || dbName_ == rhs.dbName_) && user_ == rhs.user_ &&
      password_ == rhs.password_ && specialTag_ == rhs.specialTag_ &&
      unixSocketPath_ == rhs.unixSocketPath_;
}

bool MysqlConnectionKey::operator==(const ConnectionKey& rhs) const noexcept {
  // Faster than dynamic_cast.  Only MysqlConnectionKey objects are expected
  // here.
  return *this == boost::polymorphic_downcast<const MysqlConnectionKey&>(rhs);
}

bool MysqlConnectionKey::partialEqual(const ConnectionKey& rhs) const noexcept {
  // Faster than dynamic_cast.  Only MysqlConnectionKey objects are expected
  // here.
  const auto& key = boost::polymorphic_downcast<const MysqlConnectionKey&>(rhs);
  return partialHash_ == key.partialHash_ && host_ == key.host_ &&
      port_ == key.port_ && user_ == key.user_ && password_ == key.password_ &&
      specialTag_ == key.specialTag_ && unixSocketPath_ == key.unixSocketPath_;
}

std::string MysqlConnectionKey::getDisplayString(bool level2) const {
  if (unixSocketPath_.empty()) {
    return fmt::format(
        "{} [{}] ({}@{}:{})",
        level2 ? "" : dbName_,
        specialTag_,
        user_,
        host_,
        port_);
  }

  return fmt::format(
      "{} [{}] ({}@{})",
      level2 ? "" : dbName_,
      specialTag_,
      user_,
      unixSocketPath_);
}
} // namespace mysql_client
} // namespace common
} // namespace facebook
