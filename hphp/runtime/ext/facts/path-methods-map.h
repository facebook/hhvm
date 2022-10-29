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

#include <algorithm>
#include <memory>

#include "hphp/runtime/ext/facts/autoload-db.h"
#include "hphp/runtime/ext/facts/lazy-two-way-map.h"
#include "hphp/runtime/ext/facts/path-versions.h"
#include "hphp/runtime/ext/facts/symbol-types.h"
#include "hphp/util/optional.h"

namespace HPHP {
namespace Facts {

struct PathToMethodsMap {
  using PathMethodMap = LazyTwoWayMap<Path, MethodDecl>;

  using Methods = typename PathMethodMap::Values;
  using Paths = typename PathMethodMap::Keys;

  explicit PathToMethodsMap(std::shared_ptr<PathVersions> versions)
      : m_pathMethodMap{std::move(versions)} {}

  /**
   * Return information about the locations of a given method, or the methods
   * defined in a given path.
   *
   * The const overloads will return `std::nullopt` or `nullptr` if we don't
   * yet have all the data we need to definitively answer this query. If the
   * caller receives `std::nullopt` or `nullptr`, they will need to call a
   * non-const overload to get a definitive response.
   */

  Optional<Paths> getMethodPaths(MethodDecl method) const {
    return m_pathMethodMap.getKeysForValue(method);
  }
  Paths getMethodPaths(MethodDecl method, std::vector<Path> pathsFromDB) {
    return m_pathMethodMap.getKeysForValue(method, std::move(pathsFromDB));
  }

  Optional<Methods> getPathMethods(Path path) const {
    return m_pathMethodMap.getValuesForKey(path);
  }
  Methods getPathMethods(
      Path path,
      const std::vector<AutoloadDB::MethodPath>& methodsFromDB) {
    std::vector<MethodDecl> decls;
    decls.reserve(methodsFromDB.size());
    for (auto const& [type, method, path] : methodsFromDB) {
      decls.push_back(
          {.m_type =
               {.m_name = Symbol<SymKind::Type>{type}, .m_path = Path{path}},
           .m_method = Symbol<SymKind::Function>{method}});
    }
    return m_pathMethodMap.getValuesForKey(path, std::move(decls));
  }

  /**
   * Mark the given path as containing each of the given methods, and no other
   * methods.
   */
  void replacePathMethods(Path path, Methods methods) {
    m_pathMethodMap.setValuesForKey(path, std::move(methods));
  }

 private:
  PathMethodMap m_pathMethodMap;
};

} // namespace Facts
} // namespace HPHP
