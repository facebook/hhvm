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

#include <cstdint>

#include "hphp/runtime/ext/facts/symbol-types.h"
#include "hphp/util/hash-map.h"

namespace HPHP {
namespace Facts {

/**
 * Version numbers which get bumped each time a path changes. We filter out
 * the facts in our data structures which have version numbers older than
 * the ones in this map.
 */
struct PathVersions {

  void bumpVersion(Path path) noexcept {
    always_assert(++m_versionMap[path] != 0);
  }

  std::uint64_t getVersion(Path path) const noexcept {
    auto const versionIt = m_versionMap.find(path);
    if (versionIt == m_versionMap.end()) {
      return 0;
    }
    return versionIt->second;
  }

  hphp_hash_map<Path, std::uint64_t> m_versionMap;
};

} // namespace Facts
} // namespace HPHP
