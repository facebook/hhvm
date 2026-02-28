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

#include <sys/types.h>
#include <filesystem>
#include <string>

#include "hphp/util/sqlite-wrapper.h"

namespace HPHP {
namespace Facts {

/**
 * Metadata about where a SQLite DB should be and what permissions it should
 * have.
 */
struct SQLiteKey {
  static SQLiteKey readOnly(std::filesystem::path path);
  static SQLiteKey readWrite(std::filesystem::path path);
  static SQLiteKey
  readWriteCreate(std::filesystem::path path, ::gid_t gid, ::mode_t perms);

  bool operator==(const SQLiteKey& rhs) const noexcept;

  /**
   * Render the Key as a string
   */
  std::string toString() const;
  std::string toDebugString() const;

  /**
   * Hash the Key into an int
   */
  size_t hash() const;

  std::filesystem::path m_path;
  SQLite::OpenMode m_mode;
  ::gid_t m_gid;
  ::mode_t m_perms;

 private:
  SQLiteKey() = delete;
  SQLiteKey(
      std::filesystem::path path,
      SQLite::OpenMode mode,
      ::gid_t gid,
      ::mode_t perms);
};

} // namespace Facts
} // namespace HPHP
