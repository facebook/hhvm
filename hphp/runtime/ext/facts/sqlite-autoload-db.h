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

#include <filesystem>
#include <memory>
#include <sys/types.h>

#include "hphp/runtime/ext/facts/autoload-db.h"
#include "hphp/runtime/ext/facts/sqlite-key.h"

namespace HPHP {
namespace Facts {

class SQLiteAutoloadDB : public AutoloadDB {
public:
  /**
   * Return a SQLiteAutoloadDB that can only be read
   */
  static std::shared_ptr<SQLiteAutoloadDB> readOnly(std::filesystem::path path);

  /**
   * Return a SQLiteAutoloadDB that you can write to
   */
  static std::shared_ptr<SQLiteAutoloadDB>
  readWrite(std::filesystem::path path, ::gid_t gid, ::mode_t perms);

  static std::shared_ptr<SQLiteAutoloadDB> get(const SQLiteKey& dbData);
};

} // namespace Facts
} // namespace HPHP
