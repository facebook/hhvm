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

#include <folly/experimental/io/FsUtil.h>

#include "hphp/runtime/ext/facts/autoload-db.h"
#include "hphp/runtime/ext/facts/lazy-two-way-map.h"
#include "hphp/runtime/ext/facts/symbol-types.h"

namespace HPHP {
namespace Facts {

template <typename S, SymKind k> struct PathToSymbolsMap {

  using PathSymbolMap = LazyTwoWayMap<Path<S>, Symbol<S, k>>;

  using SymbolSet = typename PathSymbolMap::ValuesSet;
  using PathSet = typename PathSymbolMap::KeysSet;

  /**
   * Return information about the locations of a given symbol, or the symbols
   * defined in a given path.
   *
   * The const overloads will return `std::nullopt` or `nullptr` if we don't
   * yet have all the data we need to definitively answer this query. If the
   * caller receives `std::nullopt` or `nullptr`, they will need to call a
   * non-const overload to get a definitive response.
   */

  const PathSet* getSymbolPaths(Symbol<S, k> symbol) const {
    return m_pathSymbolMap.getKeysForValue(symbol);
  }
  const PathSet&
  getSymbolPaths(Symbol<S, k> symbol, std::vector<Path<S>> pathsFromDB) {
    return m_pathSymbolMap.getKeysForValue(symbol, std::move(pathsFromDB));
  }

  const SymbolSet* getPathSymbols(Path<S> path) const {
    return m_pathSymbolMap.getValuesForKey(path);
  }
  const SymbolSet&
  getPathSymbols(Path<S> path, std::vector<Symbol<S, k>> symbolsFromDB) {
    return m_pathSymbolMap.getValuesForKey(path, std::move(symbolsFromDB));
  }

  /**
   * Mark the given path as no longer defined.
   */
  void removePath(Path<S> path) {
    m_pathSymbolMap.setValuesForKey(path, {});
  }

  /**
   * Mark the given path as containing each of the given symbols, and no other
   * symbols of this map's kind.
   */
  void replacePathSymbols(Path<S> path, SymbolSet symbols) {
    m_pathSymbolMap.setValuesForKey(path, std::move(symbols));
  }

private:
  PathSymbolMap m_pathSymbolMap;
};

} // namespace Facts
} // namespace HPHP
