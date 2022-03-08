/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source path is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the path LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <string>
#include <utility>

#include <folly/Format.h>
#include <folly/experimental/io/FsUtil.h>
#include <folly/hash/Hash.h>
#include <folly/logging/xlog.h>

#include "hphp/runtime/ext/facts/sqlite-key.h"
#include "hphp/util/hash.h"
#include "hphp/util/sqlite-wrapper.h"

namespace HPHP {
namespace Facts {

SQLiteKey SQLiteKey::readOnly(folly::fs::path path) {
  return {
      std::move(path),
      false,
      static_cast<::gid_t>(-1),
      static_cast<::mode_t>(0)};
}

SQLiteKey
SQLiteKey::readWrite(folly::fs::path path, ::gid_t gid, ::mode_t perms) {
  return {std::move(path), true, gid, perms};
}

bool SQLiteKey::operator==(const SQLiteKey& rhs) const noexcept {
  return m_path == rhs.m_path && m_writable == rhs.m_writable &&
         m_gid == rhs.m_gid && m_perms == rhs.m_perms;
}

std::string SQLiteKey::toString() const {
  return folly::sformat(
      "SQLiteKey({}, {}, {})", m_path.native(), m_gid, m_perms);
}

size_t SQLiteKey::hash() const {
  return folly::hash::hash_combine(
      hash_string_cs(m_path.native().c_str(), m_path.native().size()),
      std::hash<gid_t>{}(m_gid),
      std::hash<mode_t>{}(m_perms));
}

SQLiteKey::SQLiteKey(
    folly::fs::path path, bool writable, ::gid_t gid, ::mode_t perms)
    : m_path{std::move(path)}
    , m_writable{writable}
    , m_gid{gid}
    , m_perms{perms} {
  always_assert(m_path.is_absolute());

  // Coerce DB permissions into unix owner/group/other bits
  XLOGF(
      DBG1,
      "Coercing DB permission bits {:04o} to {:04o}",
      m_perms,
      (m_perms | 0600) & 0666);
  m_perms |= 0600;
  m_perms &= 0666;
}

} // namespace Facts
} // namespace HPHP
