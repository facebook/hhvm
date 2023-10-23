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
#include <re2/re2.h>
#include <vector>

#include "hphp/util/hash-map.h"
#include "hphp/util/hash-set.h"

namespace HPHP {

struct Class;
struct Func;
struct StringData;

enum class DeployKind {
  Hard,
  Soft,
  HardOrSoft,
};

struct PackageInfo {
  struct Package {
    hphp_vector_string_set m_uses;
    hphp_vector_string_set m_includes;
    hphp_vector_string_set m_soft_includes;

    template <typename SerDe> void serde(SerDe& sd) {
      sd(m_uses, stdltstr{})
        (m_includes, stdltstr{})
        (m_soft_includes, stdltstr{})
        ;
    }
  };

  using PackageSet = hphp_vector_string_set;

  struct Deployment {
    PackageSet m_packages;
    PackageSet m_soft_packages;
    std::vector<std::shared_ptr<re2::RE2>> m_domains;

    template <typename SerDe> void serde(SerDe& sd) {
      sd(m_packages, stdltstr{})
        (m_soft_packages, stdltstr{})
        ;

      std::vector<std::string> patterns;
      if constexpr (SerDe::deserializing) {
        sd(patterns);
        for (auto& s : patterns) {
          m_domains.push_back(std::make_shared<re2::RE2>(s));
        }
      } else {
        for (auto const& p : m_domains) patterns.push_back(p->pattern());
        sd(patterns);
      }
    }
  };

  using PackageMap = hphp_vector_map<std::string, Package>;
  using DeploymentMap = hphp_vector_map<std::string, Deployment>;

  const PackageMap& packages() const { return m_packages; }
  const DeploymentMap& deployments() const { return m_deployments; }

  PackageInfo(PackageMap& packages, DeploymentMap& deployments);
  PackageInfo() = default;

  const Deployment* getActiveDeployment() const;
  bool isPackageInActiveDeployment(const StringData* package) const;

  bool moduleInDeployment(const StringData* module,
                          const Deployment& deployment,
                          DeployKind deployKind) const;
  bool moduleInDeployment(const std::string& module,
                          const Deployment& deployment,
                          DeployKind deployKind) const;

  bool moduleInASoftPackage(const StringData* module) const;
  bool isModuleSoftDeployed(const StringData* module) const;

  bool violatesDeploymentBoundary(const StringData* module) const;
  bool violatesDeploymentBoundary(const Func& callee) const;
  bool violatesDeploymentBoundary(const Class& cls) const;

  std::string mangleForCacheKey() const;

  template <typename SerDe> void serde(SerDe& sd) {
    sd(m_packages, stdltstr{})
      (m_deployments, stdltstr{})
      (m_globToPackage)
      ;
  }

  static PackageInfo fromFile(const std::filesystem::path&);
  static PackageInfo defaults();

private:
  std::string findPackageInRange(const std::string& moduleName,
                                 size_t start, size_t end) const;

  std::string getPackageForModule(const StringData* module) const;

  bool moduleInPackages(const StringData* module,
                        const PackageSet& packageSet) const;

public:
  PackageMap m_packages;
  DeploymentMap m_deployments;

private:
  std::vector<std::pair<std::string, std::string>> m_globToPackage;

};

} // namespace HPHP
