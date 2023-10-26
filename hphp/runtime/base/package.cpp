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

#include "hphp/hack/src/package/ffi_bridge/package_ffi.rs.h"

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

enum class MatchKind {
  NoMatch,
  // When the glob does not match the module
  PrefixMatch,
  // When the glob prefix equals some prefix of module
  // e.g. glob = foo.bar.*
  //      module = foo.bar.baz
  PartialMatch,
  // When the glob prefix equals module
  // e.g. glob = foo.bar.*
  //      module = foo.bar
  ExactMatch,
  // When the glob equals module
  // e.g. glob = foo.bar
  //      module = foo.bar
};

MatchKind moduleNameMatchesPattern(const std::string& moduleName,
                                   const std::string& pattern) {
  if (pattern.empty()) return MatchKind::NoMatch;
  if (pattern == "*") return MatchKind::PrefixMatch;
  auto size = pattern.size();
  if (size > 2 && pattern[size-1] == '*' && pattern[size-2] == '.') {
    // Check if moduleName matches size - 2 length prefix of pattern
    if (pattern.compare(0, size - 2, moduleName) == 0) {
      return MatchKind::PartialMatch;
    }
    if (moduleName.size() <= size - 1) return MatchKind::NoMatch;
    // Check if size - 1 length prefix of moduleName matches pattern
    if (moduleName.compare(0, size - 1, pattern, 0, size - 1) == 0) {
      return MatchKind::PrefixMatch;
    }
  }
  // Looking for an exact match
  if (moduleName == pattern) return MatchKind::ExactMatch;
  return MatchKind::NoMatch;
}

} // namespace

// Given `moduleName`, find the longest matching glob within
// m_globToPackage[start:end) and return its corresponding package
std::string PackageInfo::findPackageInRange(const std::string& moduleName,
                                            size_t start, size_t end) const {
  if (start >= end) return "";
  size_t mid = start + (end - start) / 2;
  auto const& glob = m_globToPackage[mid].first;
  auto const match = moduleNameMatchesPattern(moduleName, glob);

  if (match ==  MatchKind::NoMatch) {
    // attempt to find a more specific glob match in the upper half first
    auto const package = findPackageInRange(moduleName, mid + 1, end);
    if (!package.empty()) return package;
    // fall back to finding a match in the lower half
    return findPackageInRange(moduleName, start, mid);
  }

  auto const& currentMatchedPackage = m_globToPackage[mid].second;
  if (match == MatchKind::ExactMatch) return currentMatchedPackage;
  if (match == MatchKind::PrefixMatch) {
    // prioritize a more specific glob match if one exists
    auto const nextMatchedPackage = findPackageInRange(moduleName, mid + 1, end);
    if (!nextMatchedPackage.empty()) return nextMatchedPackage;
    return currentMatchedPackage;
  }

  assertx(match == MatchKind::PartialMatch);
  // check whether the glob immediately before "<moduleName>.*" could
  // be an exact match, and if so, return it instead
  if (mid - 1 >= 0) {
    auto const& prev = m_globToPackage[mid - 1];
    if (moduleNameMatchesPattern(moduleName, prev.first) == MatchKind::ExactMatch) {
      return prev.second;
    }
  }
  return currentMatchedPackage;
}

std::string PackageInfo::getPackageForModule(const StringData* module) const {
  assertx(module && !module->empty());
  auto const moduleName = module->toCppString();
  return findPackageInRange(moduleName, 0, m_globToPackage.size());
}

bool PackageInfo::moduleInPackages(const StringData* module,
                                   const PackageSet& packageSet) const {
  auto const packageForModule = getPackageForModule(module);
  return packageSet.contains(packageForModule);
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
  if (!module || module->empty()) return false;
  if (RO::RepoAuthoritative) return isModuleSoftDeployed(module);
  for (auto& [_, deployment] : deployments()) {
    if (moduleInDeployment(module, deployment, DeployKind::Soft)) {
      return true;
    }
  }
  return false;
}

namespace {

hphp_fast_map<const StringData*, bool> m_moduleInActiveDeployment;
folly::SharedMutex s_mutex;

} // namespace

bool PackageInfo::isModuleSoftDeployed(const StringData* module) const {
  if (!module || module->empty()) return false;
  if (auto const activeDeployment = getActiveDeployment()) {
    return moduleInDeployment(module, *activeDeployment, DeployKind::Soft);
  }
  return false;
}

bool PackageInfo::violatesDeploymentBoundary(const StringData* module) const {
  if (!RO::EvalEnforceDeployment) return false;
  if (auto const activeDeployment = getActiveDeployment()) {
    if (RO::RepoAuthoritative) {
      folly::SharedMutex::ReadHolder lock(s_mutex);
      auto it = m_moduleInActiveDeployment.find(module);
      if (it != m_moduleInActiveDeployment.end()) return !it->second;
    }
    auto const inActiveDeployment = moduleInDeployment(
      module, *activeDeployment, DeployKind::Hard);
    if (RO::RepoAuthoritative) {
      folly::SharedMutex::WriteHolder lock(s_mutex);
      m_moduleInActiveDeployment.emplace(module, inActiveDeployment);
    }
    return !inActiveDeployment;
  }
  return false;
}

bool PackageInfo::violatesDeploymentBoundary(const Func& callee) const {
  if (!RO::EvalEnforceDeployment) return false;
  if (RO::RepoAuthoritative) {
    return callee.unit()->isSoftDeployedRepoOnly();
  }
  return violatesDeploymentBoundary(callee.moduleName());
}

bool PackageInfo::violatesDeploymentBoundary(const Class& cls) const {
  if (!RO::EvalEnforceDeployment) return false;
  if (RO::RepoAuthoritative) {
    return cls.preClass()->unit()->isSoftDeployedRepoOnly();
  }
  return violatesDeploymentBoundary(cls.moduleName());
}

} // namespace HPHP
