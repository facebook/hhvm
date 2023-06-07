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

  return PackageInfo { packages, deployments };
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

  return folly::toJson(result);
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
    // Looking for a prefix match
    // TODO: This is very inefficient, improve this using a something along the
    // lines of a trie before using it in production
    if (moduleName.size() <= size - 1) return false;
    // Check if size - 1 length prefix of moduleName matches pattern
    return moduleName.compare(0, size - 1, pattern, 0, size - 1) == 0;
  }
  // Looking for an exact match
  return moduleName == pattern;
}

} // namespace

bool PackageInfo::moduleInPackage(const StringData* module,
                                  const Package& package) const {
  assertx(module && !module->empty());
  auto const moduleName = module->toCppString();
  for (auto const& pattern : package.m_uses) {
    if (moduleNameMatchesPattern(moduleName, pattern)) return true;
  }
  return false;
}

bool PackageInfo::moduleInDeployment(const StringData* module,
                                     const Deployment& deployment,
                                     bool allow_soft /* = true */) const {
  assertx(module && !module->empty());
  for (auto const& package : deployment.m_packages) {
    auto const it = packages().find(package);
    if (it == end(packages())) continue;
    if (moduleInPackage(module, it->second)) return true;
  }
  if (allow_soft) {
    for (auto const& package : deployment.m_soft_packages) {
      auto const it = packages().find(package);
      if (it == end(packages())) continue;
      if (moduleInPackage(module, it->second)) return true;
    }
  }
  return false;
}

namespace {

bool is_module_outside_active_deployment(const PackageInfo& packageInfo,
                                         const StringData* module) {
  if (!RO::EvalEnforceDeployment) return false;
  if (auto const activeDeployment = packageInfo.getActiveDeployment()) {
    if (!module) return true;
    return !packageInfo.moduleInDeployment(module, *activeDeployment);
  }
  return false;
}

} // namespace

bool will_symbol_raise_deployment_boundary_violation(const PackageInfo& packageInfo,
                                                     const Func* callee) {
  assertx(callee);
  if (callee->unit()->isSystemLib()) return false;
  return is_module_outside_active_deployment(packageInfo, callee->moduleName());
}

bool will_symbol_raise_deployment_boundary_violation(const PackageInfo& packageInfo,
                                                     const Class* cls) {
  assertx(cls);
  return is_module_outside_active_deployment(packageInfo, cls->moduleName());
}

} // namespace HPHP
