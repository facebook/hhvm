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
#include <regex>
#include <vector>

#include "hphp/util/hash-map.h"

namespace HPHP {

struct PackageInfo {
  struct Package {
    std::vector<std::string> m_uses;
    std::vector<std::string> m_includes;
  };

  struct Deployment {
    std::vector<std::string> m_packages;
    std::vector<std::regex> m_domains;
    // Need to maintain the string form of regex for cache key mangling
    // C++ does not provide a way to convert a regex back to string
    std::vector<std::string> m_domainsOriginal;
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
