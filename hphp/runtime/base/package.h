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

#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <re2/re2.h>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <folly/container/HeterogeneousAccess.h>

#include "hphp/util/hash-map.h"
#include "hphp/util/hash-set.h"
#include "hphp/util/tiny-vector.h"

namespace HPHP {

struct Class;
struct Func;
struct StringData;

enum class DeployKind {
  Hard,
  Soft,
  NotDeployed,
};

struct PackageInfo {
  struct ResolvedPackagePolicy {
    bool strictIsolation{false};
    bool raiseDynamicClassLoadError{false};
  };

  struct Package {
    hphp_vector_string_set m_includes;
    hphp_vector_string_set m_soft_includes;
    hphp_vector_string_set m_include_paths;
    bool m_enable_strict_isolation{false};
    bool m_raiseDynamicClassLoadError{false};

    template <typename SerDe> void serde(SerDe& sd) {
      sd(m_includes, stdltstr{})
        (m_soft_includes, stdltstr{})
        (m_include_paths, stdltstr{})
        (m_enable_strict_isolation)
        (m_raiseDynamicClassLoadError)
        ;
    }
  };

  using PackageSet = hphp_vector_string_set;

  struct Deployment {
    PackageSet m_packages;
    PackageSet m_soft_packages;

    template <typename SerDe> void serde(SerDe& sd) {
      sd(m_packages, stdltstr{})
        (m_soft_packages, stdltstr{})
        ;
    }

    DeployKind getDeployKind(const std::string& package) const {
      if (m_packages.contains(package)) {
        return DeployKind::Hard;
      } else if (m_soft_packages.contains(package)) {
        return DeployKind::Soft;
      } else {
        return DeployKind::NotDeployed;
      }
    }
  };

  struct ImplicitPackageFamily {
    std::string m_path;
    PackageSet m_includes;
    PackageSet m_soft_includes;
    bool m_raiseDynamicClassLoadError{false};

    template <typename SerDe> void serde(SerDe& sd) {
      sd(m_path)
        (m_includes, stdltstr{})
        (m_soft_includes, stdltstr{})
        (m_raiseDynamicClassLoadError)
        ;
    }
  };

  using PackageMap = hphp_vector_map<std::string, Package>;
  using DeploymentMap = hphp_vector_map<std::string, Deployment>;
  using ImplicitPackageFamilyMap = hphp_vector_map<
    std::string,
    ImplicitPackageFamily,
    folly::HeterogeneousAccessHash<std::string>,
    folly::HeterogeneousAccessEqualTo<std::string>
  >;

  struct PackageOrImplicitFamilyPath {
    std::string m_path;
    std::string m_package;
    bool m_isImplicit;

    template <typename SerDe> void serde(SerDe& sd) {
      sd(m_path)(m_package)(m_isImplicit);
    }
  };

  using PackageAndImplicitFamilyPathsInLookupOrder =
    std::vector<PackageOrImplicitFamilyPath>;

  const PackageMap& packages() const { return m_packages; }
  const DeploymentMap& deployments() const { return m_deployments; }
  const ImplicitPackageFamilyMap& implicitPackageFamilies() const {
    return m_implicitPackageFamilies;
  }
  const PackageAndImplicitFamilyPathsInLookupOrder&
  packageAndImplicitFamilyPathsInLookupOrder() const {
    return m_packageAndImplicitFamilyPathsInLookupOrder;
  }
  ResolvedPackagePolicy resolvePackagePolicy(
    const std::string& package
  ) const;
  ResolvedPackagePolicy strictDynamicReferencePolicyForPath(
    std::filesystem::path path,
    const std::filesystem::path& repoRoot
  ) const;

  PackageInfo() = default;

  const Deployment* getActiveDeployment() const;
  bool implPackageExists(const StringData* package) const;

  std::optional<std::string>
  implicitPackageNameToPathPrefix(std::string_view name) const;
  std::optional<std::string>
  pathToPackageName(std::string_view path) const;

  std::string mangleForCacheKey() const;

  template <typename SerDe> void serde(SerDe& sd) {
    sd(m_packages, stdltstr{})
      (m_deployments, stdltstr{})
      (m_implicitPackageFamilies, stdltstr{})
      (m_packageAndImplicitFamilyPathsInLookupOrder)
      ;
  }

  static PackageInfo fromFile(const std::filesystem::path&,
                              bool enableImplicitPackages);
  static PackageInfo defaults();

public:
  PackageMap m_packages;
  DeploymentMap m_deployments;
  ImplicitPackageFamilyMap m_implicitPackageFamilies;

private:
  PackageInfo(const PackageMap& packages,
              const DeploymentMap& deployments,
              const ImplicitPackageFamilyMap& implicitPackageFamilies,
              PackageAndImplicitFamilyPathsInLookupOrder
                packageAndImplicitFamilyPathsInLookupOrder);

  PackageAndImplicitFamilyPathsInLookupOrder
    m_packageAndImplicitFamilyPathsInLookupOrder;
};

} // namespace HPHP
