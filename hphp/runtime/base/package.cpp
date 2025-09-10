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

#include "hphp/runtime/base/configs/eval-loader.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/server/cli-server.h"
#include "hphp/runtime/vm/func.h"

#include "hphp/util/configs/eval.h"

#include <re2/re2.h>
#include "hphp/util/rds-local.h"

#include "hphp/hack/src/package/ffi_bridge/package_ffi.rs.h"

#include <folly/json/dynamic.h>
#include <folly/json/json.h>
#include <fstream>

namespace HPHP {

// Most patterns are the same, so de-dup them and avoid creating
// re2::RE2 instances.

namespace {

hphp_fast_map<std::string, std::unique_ptr<const re2::RE2>> s_patternCache;
folly::SharedMutex s_patternCacheLock;

}

const re2::RE2& PackageInfo::compilePattern(const std::string& p) {
  {
    std::shared_lock _{s_patternCacheLock};
    if (auto const re = folly::get_ptr(s_patternCache, p)) return **re;
  }
  std::unique_lock _{s_patternCacheLock};
  if (auto const re = folly::get_ptr(s_patternCache, p)) return **re;
  return
    *s_patternCache.try_emplace(p, std::make_unique<re2::RE2>(p)).first->second;
}

PackageInfo::PackageInfo(PackageMap& packages,
                         DeploymentMap& deployments)
  : m_packages(packages)
  , m_deployments(deployments) {}

PackageInfo PackageInfo::fromFile(const std::filesystem::path& path) {
  PackageMap packages;
  DeploymentMap deployments;

  try {
    if (!std::filesystem::exists(path)) {
      if (Cfg::Eval::PackagesTomlFileName != Cfg::EvalLoader::PackagesTomlFileNameDefault()) {
        Logger::Warning(
          "Could not open the package specification: %s. Continuing with the empty package specification.",
          path.string().c_str());
      }
      return defaults();
    }

    auto info = package::package_info(path.string());

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
                         convert(p.package.includes),
                         convert(p.package.soft_includes),
                         convert(p.package.include_paths)
                       });
    }

    for (auto& d : info.deployments) {
      deployments.emplace(std::string(d.name),
                          Deployment {
                            convert(d.deployment.packages),
                            convert(d.deployment.soft_packages),
                          });
    }
    if (info.errors.size() > 0) {
      std::vector<folly::StringPiece> packageConfigErrors;
      for (auto& error : info.errors) {
        packageConfigErrors.push_back(error.c_str());
      }
      auto const packageConfigError = folly::sformat(
        "Error parsing {}: {}",
        path.c_str(),
        folly::join("\n", packageConfigErrors)
      );
      Logger::FError(packageConfigError);
    }

    return PackageInfo(packages, deployments);
  } catch (const std::exception& e) {
    Logger::Warning(
      "Exception %s when reading: %s. Continuing with the empty package specification.",
      e.what(), path.c_str());
    return defaults();
  }
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
  const TinyVector<const re2::RE2*>& data
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
    entry["include_paths"] = mangleVecForCacheKey(package.m_include_paths);
    entry["includes"] = mangleVecForCacheKey(package.m_includes);
    entry["soft_includes"] = mangleVecForCacheKey(package.m_soft_includes);
    result[name] = entry;
  }

  for (auto& [name, deployment] : deployments()) {
    folly::dynamic entry = folly::dynamic::object();
    entry["packages"] = mangleVecForCacheKey(deployment.m_packages);
    entry["soft_packages"] = mangleVecForCacheKey(deployment.m_soft_packages);
    result[name] = entry;
  }

  // By default the ordering of keys in dynamic objects is unspecified, and
  // in dbg builds we randomize the order to ensure no one is depending on it.
  folly::json::serialization_opts opts;
  opts.sort_keys = true;
  return folly::json::serialize(std::move(result), std::move(opts));
}

static RDS_LOCAL_NO_CHECK(const PackageInfo::Deployment*, s_requestActiveDeployment);

const PackageInfo::Deployment* PackageInfo::getActiveDeployment() const {
  auto const findDeploymentByName = [&](const std::string& name) -> const PackageInfo::Deployment* {
    auto const it = deployments().find(name);
    if (it == end(deployments())) return nullptr;
    return &it->second;
  };

  if (Cfg::Repo::Authoritative || !Cfg::Server::Mode) {
    return findDeploymentByName(Cfg::Eval::ActiveDeployment);
  }
  // If unset, set the cached active deployment to null by default.
  if (s_requestActiveDeployment.isNull()) {
    auto const activeDeployment = [&]() -> const PackageInfo::Deployment* {
        // If we're in the CLI server, get the active deployment from cli.hdf
        // Otherwise, read the active deployment from config.hdf
        return is_cli_server_mode() ? findDeploymentByName(cli_get_active_deployment())
                                    : findDeploymentByName(Cfg::Eval::ActiveDeployment);
    }();
    s_requestActiveDeployment.emplace(activeDeployment);
  }
  assertx(!s_requestActiveDeployment.isNull());
  return *s_requestActiveDeployment;
}

bool PackageInfo::implPackageExists(const StringData* package) const {
  assertx(package);
  if (package->empty()) return false;
  auto const activeDeployment = getActiveDeployment();
  // If there's no active deployment, return whether package exists at all
  if (!activeDeployment) return packages().contains(package->toCppString());
  switch (activeDeployment->getDeployKind(package->toCppString())) {
    case DeployKind::Hard:
    case DeployKind::Soft:
    case DeployKind::HardOrSoft:
      return true;
    case DeployKind::NotDeployed:
      return false;
  }
}

} // namespace HPHP
