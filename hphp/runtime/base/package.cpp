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
  DeploymentMap deployments;

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

  for (auto& d : info.deployments) {
    std::vector<std::string> packages, domainsOriginal;
    std::vector<std::regex> domains;
    for (auto& s : d.deployment.packages) packages.push_back(std::string(s));
    for (auto& s : d.deployment.domains) {
      domains.push_back(std::regex(std::string(s)));
      domainsOriginal.push_back(std::string(s));
    }
    deployments.emplace(std::string(d.name),
                        Deployment { packages, domains, domainsOriginal });
  }
#endif

  return PackageInfo { packages, deployments };
}

PackageInfo PackageInfo::defaults() {
  // Empty package for now
  return {};
}

namespace {
folly::dynamic mangleVecForCacheKey(const std::vector<std::string>& vec) {
  folly::dynamic result = folly::dynamic::array();
  for (auto& s : vec) result.push_back(s);
  return result;
}
} // namespace

std::string PackageInfo::mangleForCacheKey() const {
  folly::dynamic result = folly::dynamic::object();

  for (auto& [name, package] : packages()) {
    folly::dynamic entry = folly::dynamic::object();
    entry["uses"] = mangleVecForCacheKey(package.m_uses);
    entry["includes"] = mangleVecForCacheKey(package.m_includes);
    result[name] = entry;
  }

  for (auto& [name, deployment] : deployments()) {
    folly::dynamic entry = folly::dynamic::object();
    entry["packages"] = mangleVecForCacheKey(deployment.m_packages);
    entry["domains"] = mangleVecForCacheKey(deployment.m_domainsOriginal);
    result[name] = entry;
  }

  return folly::toJson(result);
}

} // namespace HPHP
