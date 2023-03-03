/*
 +----------------------------------------------------------------------+
 | HipHop for PHP                                                       |
 +----------------------------------------------------------------------+
 | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
 +----------------------------------------------------------------------+
 | This source file is subject to version 3.01 of the PHP license,      |
 | that is bundled with this package in the file LICENSE, and is        |
 | available through the world-wide-web at the following url:           |
 | http://www.php.net/license/3_01.txt                                  |
 | If you did not receive a copy of the PHP license and are unable to   |
 | obtain it through the world-wide-web, please send a note to          |
 | license@php.net so we can mail you a copy immediately.               |
 +----------------------------------------------------------------------+
*/

#include "hphp/runtime/base/package.h"

//TODO(T146965521) Until Rust FFI symbol redefinition problem can be resolved
#ifdef FACEBOOK
#include "hphp/hack/src/package/ffi_bridge/package_ffi.rs.h"
#endif

#include <folly/dynamic.h>
#include <folly/json.h>
#include <fstream>

namespace HPHP {

PackageInfo PackageInfo::fromFile(const std::filesystem::path& path) {
  std::ifstream file(path, std::ios::in);
  if (!file.is_open()) return {};

  PackageMap packages;

//TODO(T146965521) Until Rust FFI symbol redefinition problem can be resolved
#ifdef FACEBOOK
  std::string packages_toml{
    std::istreambuf_iterator<char>(file),
    std::istreambuf_iterator<char>()
  };

  auto info = package::package_info_cpp_ffi(packages_toml);

  for (auto& p : info.packages) {
    std::vector<std::string> uses, includes;
    for (auto& s : p.package.uses) uses.push_back(std::string(s));
    for (auto& s : p.package.includes) includes.push_back(std::string(s));
    packages.emplace(std::string(p.name), Package { uses, includes });
  }
#endif

  return PackageInfo { packages };
}

PackageInfo PackageInfo::defaults() {
  // Empty package for now
  return {};
}

std::string PackageInfo::mangleForCacheKey() const {
  folly::dynamic result = folly::dynamic::object();

  for (auto& [name, package] : packages()) {
    folly::dynamic entry = folly::dynamic::object();

    folly::dynamic uses = folly::dynamic::array();
    for (auto& s : package.m_uses) uses.push_back(s);
    entry["uses"] = uses;

    folly::dynamic includes = folly::dynamic::array();
    for (auto& s : package.m_includes) includes.push_back(s);
    entry["includes"] = includes;

    result[name] = entry;
  }

  return folly::toJson(result);
}

} // namespace HPHP
