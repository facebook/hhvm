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

struct PackageInfo {
  struct Package {
    hphp_vector_string_set m_uses;
    hphp_vector_string_set m_includes;
  };

  struct Deployment {
    hphp_vector_string_set m_packages;
    std::vector<std::shared_ptr<re2::RE2>> m_domains;
  };

  using PackageMap = hphp_vector_map<std::string, Package>;
  using DeploymentMap = hphp_vector_map<std::string, Deployment>;

  const PackageMap& packages() const { return m_packages; }
  const DeploymentMap& deployments() const { return m_deployments; }

  std::string mangleForCacheKey() const;

  static PackageInfo fromFile(const std::filesystem::path&);
  static PackageInfo defaults();

  PackageMap m_packages;
  DeploymentMap m_deployments;
};

} // namespace HPHP
