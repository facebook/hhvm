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

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/func.h"

#include <re2/re2.h>
#include "hphp/util/trace.h"

//TODO(T146965521) Until Rust FFI symbol redefinition problem can be resolved
#ifdef HHVM_FACEBOOK
#include "hphp/hack/src/package/ffi_bridge/package_ffi.rs.h"
#endif

#include <folly/dynamic.h>
#include <folly/json.h>
#include <fstream>

namespace HPHP {


PackageInfo::PackageInfo(PackageMap& packages,
                         DeploymentMap& deployments)
  : m_packages(packages)
  , m_deployments(deployments) {
  for (auto const& [packageName, package] : m_packages) {
    for (auto const& glob: package.m_uses) {
      m_globToPackage.push_back(std::make_pair(glob, packageName));
    }
  }
  std::sort(m_globToPackage.begin(), m_globToPackage.end());
}

PackageInfo PackageInfo::fromFile(const std::filesystem::path& path) {
  std::ifstream file(path, std::ios::in);
  if (!file.is_open()) return defaults();

  PackageMap packages;
  DeploymentMap deployments;

//TODO(T146965521) Until Rust FFI symbol redefinition problem can be resolved
#ifdef HHVM_FACEBOOK
  std::string packages_toml{
    std::istreambuf_iterator<char>(file),
    std::istreambuf_iterator<char>()
  };

  auto info = package::package_info(packages_toml);

  auto const convert = [&] (auto const& v) {
    hphp_vector_string_set result;
    // hphp_vector_string_set inserts to the beginning instead of to the end,
    // insert in reverse order to make up for this.
    // rust::Vec does not define rbegin and rend.
    for (size_t i = v.size(); i > 0; --i) {
      result.insert(std::string(v[i-1]));
    }
    return result;
  };

  for (auto& p : info.packages) {
    packages.emplace(std::string(p.name),
                     Package {
                       convert(p.package.uses),
                       convert(p.package.includes),
                       convert(p.package.soft_includes)
                     });
  }

  for (auto& d : info.deployments) {
    std::vector<std::shared_ptr<re2::RE2>> domains;
    for (auto& s : d.deployment.domains) {
      domains.push_back(std::make_shared<re2::RE2>(std::string(s)));
    }
    deployments.emplace(std::string(d.name),
                        Deployment {
                          convert(d.deployment.packages),
                          convert(d.deployment.soft_packages),
                          std::move(domains),
                        });
  }
#endif

  return PackageInfo(packages, deployments);
}

PackageInfo PackageInfo::defaults() {
  // Empty package for now
  return {};
}

namespace {
folly::dynamic mangleVecForCacheKey(const hphp_vector_string_set& data) {
  folly::dynamic result = folly::dynamic::array();
  for (auto& s : data) result.push_back(s);
  return result;
}

folly::dynamic mangleVecForCacheKey(
  const std::vector<std::shared_ptr<re2::RE2>>& data
) {
  folly::dynamic result = folly::dynamic::array();
  for (auto& r : data) result.push_back(r->pattern());
  return result;
}
} // namespace

std::string PackageInfo::mangleForCacheKey() const {
  folly::dynamic result = folly::dynamic::object();

  for (auto& [name, package] : packages()) {
    folly::dynamic entry = folly::dynamic::object();
    entry["uses"] = mangleVecForCacheKey(package.m_uses);
    entry["includes"] = mangleVecForCacheKey(package.m_includes);
    entry["soft_includes"] = mangleVecForCacheKey(package.m_soft_includes);
    result[name] = entry;
  }

  for (auto& [name, deployment] : deployments()) {
    folly::dynamic entry = folly::dynamic::object();
    entry["packages"] = mangleVecForCacheKey(deployment.m_packages);
    entry["soft_packages"] = mangleVecForCacheKey(deployment.m_soft_packages);
    entry["domains"] = mangleVecForCacheKey(deployment.m_domains);
    result[name] = entry;
  }

  // By default the ordering of keys in dynamic objects is unspecified, and
  // in dbg builds we randomize the order to ensure no one is depending on it.
  folly::json::serialization_opts opts;
  opts.sort_keys = true;
  return folly::json::serialize(std::move(result), std::move(opts));
}

const PackageInfo::Deployment* PackageInfo::getActiveDeployment() const {
  if (RO::RepoAuthoritative || !RuntimeOption::ServerExecutionMode()) {
    auto const it = deployments().find(RO::EvalActiveDeployment);
    if (it == end(deployments())) return nullptr;
    return &it->second;
  }
  if (!g_context || !g_context->getTransport()) return nullptr;
  auto const host = g_context->getTransport()->getHeader("Host");
  for (auto const& [_, deployment]: deployments()) {
    for (auto const& domain: deployment.m_domains) {
      if (re2::RE2::FullMatch(host, *domain)) return &deployment;
    }
  }
  return nullptr;
}

bool PackageInfo::isPackageInActiveDeployment(const StringData* package) const {
  if (!package || package->empty()) return false;
  auto const activeDeployment = getActiveDeployment();
  // If there's no active deployment, return whether package exists at all
  if (!activeDeployment) return packages().contains(package->toCppString());
  return activeDeployment->m_packages.contains(package->toCppString());
}

namespace {

bool moduleNameMatchesPattern(const std::string& moduleName,
                              const std::string& pattern) {
  if (pattern.empty()) return false;
  if (pattern == "*") return true;
  auto size = pattern.size();
  if (size > 2 && pattern[size-1] == '*' && pattern[size-2] == '.') {
    if (moduleName.size() <= size - 1) return false;
    // Check if size - 1 length prefix of moduleName matches pattern
    return moduleName.compare(0, size - 1, pattern, 0, size - 1) == 0;
  }
  // Looking for an exact match
  return moduleName == pattern;
}

} // namespace

// Given `moduleName`, find the longest matching glob within
// m_globToPackage[start:end) and return its corresponding package
std::string PackageInfo::findPackageInRange(const std::string& moduleName,
                                            size_t start, size_t end) const {
  if (start >= end) return "";
  size_t mid = start + (end - start) / 2;
  auto const& glob = m_globToPackage[mid].first;

  // impossible to match against globs that are lexicographically larger
  if (glob > moduleName) {
    return findPackageInRange(moduleName, start, mid);
  }

  if (moduleNameMatchesPattern(moduleName, glob)) {
    auto const& currentMatch = m_globToPackage[mid].second;
    // prioritize a more specific glob match if one exists
    auto const nextMatch = findPackageInRange(moduleName, mid + 1, end);
    if (!nextMatch.empty()) return nextMatch;
    return currentMatch;
  }
  // attempt to find a glob match in the upper half first
  auto const match = findPackageInRange(moduleName, mid + 1, end);
  if (!match.empty()) return match;
  // fall back to finding a match in the lower half
  return findPackageInRange(moduleName, start, mid);
}

std::string PackageInfo::getPackageForModule(const StringData* module) const {
  assertx(module && !module->empty());
  auto const moduleName = module->toCppString();
  return findPackageInRange(moduleName, 0, m_globToPackage.size());
}

bool PackageInfo::moduleInPackages(const StringData* module,
                                   const PackageSet& packageSet) const {
  auto const packageForModule = getPackageForModule(module);
  for (auto const& package : packageSet) {
    auto const it = packages().find(package);
    if (it == end(packages())) continue;
    if (package == packageForModule) return true;
  }
  return false;
}

bool PackageInfo::moduleInDeployment(const StringData* module,
                                     const Deployment& deployment,
                                     DeployKind deployKind) const {
  assertx(module && !module->empty());
  if (deployKind == DeployKind::Hard) {
    return moduleInPackages(module, deployment.m_packages);
  }
  if (deployKind == DeployKind::Soft) {
    return moduleInPackages(module, deployment.m_soft_packages);
  }
  return moduleInPackages(module, deployment.m_packages) ||
         moduleInPackages(module, deployment.m_soft_packages);
}

bool PackageInfo::moduleInASoftPackage(const StringData* module) const {
  if (!module  || module->empty()) return false;
  for (auto& [_, deployment] : deployments()) {
    if (moduleInDeployment(module, deployment, DeployKind::Soft)) {
      return true;
    }
  }
  return false;
}

bool PackageInfo::outsideActiveDeployment(const StringData* module) const {
  if (!RO::EvalEnforceDeployment) return false;
  if (!module || module->empty()) return false;
  if (auto const activeDeployment = getActiveDeployment()) {
    return !moduleInDeployment(module, *activeDeployment, DeployKind::Hard);
  }
  return false;
}

bool PackageInfo::outsideActiveDeployment(const Func& callee) const {
  return outsideActiveDeployment(callee.moduleName());
}

bool PackageInfo::outsideActiveDeployment(const Class& cls) const {
  return outsideActiveDeployment(cls.moduleName());
}

} // namespace HPHP
