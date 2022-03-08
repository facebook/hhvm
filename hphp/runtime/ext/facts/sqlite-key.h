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

#pragma once

#include <string>
#include <sys/types.h>

#include <folly/experimental/io/FsUtil.h>

namespace HPHP {
namespace Facts {

/**
 * Metadata about where a SQLite DB should be and what permissions it should
 * have.
 */
struct SQLiteKey {

  static SQLiteKey readOnly(folly::fs::path path);
  static SQLiteKey readWrite(folly::fs::path path, ::gid_t gid, ::mode_t perms);

  bool operator==(const SQLiteKey& rhs) const noexcept;

  /**
   * Render the Key as a string
   */
  std::string toString() const;

  /**
   * Hash the Key into an int
   */
  size_t hash() const;

  folly::fs::path m_path;
  bool m_writable;
  ::gid_t m_gid;
  ::mode_t m_perms;

private:
  SQLiteKey() = delete;
  SQLiteKey(folly::fs::path path, bool writable, ::gid_t gid, ::mode_t perms);
};

} // namespace Facts
} // namespace HPHP
