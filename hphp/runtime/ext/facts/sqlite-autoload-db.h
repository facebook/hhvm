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

#include <iterator>
#include <utility>

#include <folly/experimental/io/FsUtil.h>

#include "hphp/runtime/ext/facts/autoload-db.h"
#include "hphp/runtime/ext/facts/file-facts.h"
#include "hphp/util/sqlite-wrapper.h"

namespace HPHP {
namespace Facts {

class SQLiteAutoloadDB : public AutoloadDB {
public:
  /**
   * Metadata about where the DB should be and what permissions it should have.
   */
  struct Key {

    static Key readOnly(folly::fs::path path) {
      return Key{
          std::move(path),
          SQLite::OpenMode::ReadOnly,
          static_cast<::gid_t>(-1),
          0};
    }

    static Key readWrite(folly::fs::path path, ::gid_t gid, ::mode_t perms) {
      return Key{std::move(path), SQLite::OpenMode::ReadWrite, gid, perms};
    }

    bool operator==(const Key& rhs) const;

    /**
     * Render the Key as a string
     */
    std::string toString() const;

    /**
     * Hash the Key into an int
     */
    size_t hash() const;

    folly::fs::path m_path;
    SQLite::OpenMode m_rwMode;
    ::gid_t m_gid;
    ::mode_t m_perms;

  private:
    Key() = delete;
    Key(folly::fs::path path,
        SQLite::OpenMode rwMode,
        ::gid_t gid,
        ::mode_t perms);
  };

  /**
   * Return a SQLiteAutoloadDB that can only be read
   */
  static std::unique_ptr<SQLiteAutoloadDB> readOnly(folly::fs::path path);

  /**
   * Return a SQLiteAutoloadDB that you can write to
   */
  static std::unique_ptr<SQLiteAutoloadDB>
  readWrite(folly::fs::path path, ::gid_t gid, ::mode_t perms);

  static SQLiteAutoloadDB& getThreadLocal(const Key& dbData);
};

} // namespace Facts
} // namespace HPHP
