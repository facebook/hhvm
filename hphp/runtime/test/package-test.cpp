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

#include <folly/testing/TestUtil.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <tuple>

namespace HPHP {
namespace {

// Checks that config loading gates implicit families and reports strict status.
TEST(PackageInfoTest, LoadsImplicitPackageFamiliesOnlyWhenEnabled) {
  folly::test::TemporaryDirectory temp{"package-info"};
  auto const config = temp.path() / "PACKAGES.toml";
  std::ofstream out{config};
  out << R"(
[packages]
[packages.intern]
include_paths = ["//alpha/", "//www/"]
[packages.soft]

[packages.strict]
enable_strict_isolation = true

[implicit_packages.prototypes]
path = "//www/prototypes/"
includes = ["intern"]
soft_includes = ["soft"]
)";
  out.close();

  auto const configPath = std::filesystem::path{config.native()};
  auto const enabled = PackageInfo::fromFile(configPath, true);
  ASSERT_EQ(enabled.implicitPackageFamilies().size(), 1);
  auto const& family = enabled.implicitPackageFamilies().at("prototypes");
  EXPECT_EQ(family.m_path, "www/prototypes/");
  EXPECT_TRUE(family.m_includes.contains("intern"));
  EXPECT_TRUE(family.m_soft_includes.contains("soft"));
  EXPECT_FALSE(enabled.packages().contains("prototypes"));

  auto const disabled = PackageInfo::fromFile(configPath, false);
  EXPECT_TRUE(disabled.implicitPackageFamilies().empty());
  EXPECT_TRUE(disabled.packages().contains("intern"));
  EXPECT_TRUE(disabled.packages().contains("soft"));
  EXPECT_TRUE(enabled.isStrictIsolationPackage("strict"));
  EXPECT_FALSE(enabled.isStrictIsolationPackage("intern"));
  EXPECT_TRUE(enabled.isStrictIsolationPackage("prototypes"));
  EXPECT_TRUE(enabled.isStrictIsolationPackage("prototypes.example"));
  EXPECT_FALSE(enabled.isStrictIsolationPackage("unknown"));

  std::vector<std::tuple<std::string, std::string, bool>>
    packageAndImplicitFamilyPathsInLookupOrder;
  for (auto const& entry :
       enabled.packageAndImplicitFamilyPathsInLookupOrder()) {
    packageAndImplicitFamilyPathsInLookupOrder.emplace_back(
      entry.m_path,
      entry.m_package,
      entry.m_isImplicit
    );
  }
  EXPECT_EQ(
    packageAndImplicitFamilyPathsInLookupOrder,
    (std::vector<std::tuple<std::string, std::string, bool>>{
      {"www/prototypes/", "prototypes", true},
      {"www/", "intern", false},
      {"alpha/", "intern", false},
    })
  );
}

// Checks that strict-isolation metadata contributes to the package cache key.
TEST(PackageInfoTest, StrictIsolationChangesCacheMangle) {
  PackageInfo base;
  base.m_packages.emplace("example", PackageInfo::Package{});

  auto strict = base;
  strict.m_packages.at("example").m_enable_strict_isolation = true;

  EXPECT_NE(base.mangleForCacheKey(), strict.mangleForCacheKey());
}

// Checks that the cache key includes an empty implicit package family map.
TEST(PackageInfoTest, MangleIncludesEmptyImplicitFamilies) {
  PackageInfo info;
  info.m_packages.emplace("example", PackageInfo::Package{});

  EXPECT_EQ(
    info.mangleForCacheKey(),
    R"([{"example":{"enable_strict_isolation":false,"include_paths":[],"includes":[],"soft_includes":[]}},{}])"
  );
}

// Checks that cache keys store explicit packages and implicit families separately.
TEST(PackageInfoTest, SeparatesImplicitFamiliesInCacheMangle) {
  PackageInfo info;
  info.m_packages.emplace("implicit_families", PackageInfo::Package{});
  info.m_implicitPackageFamilies.emplace(
    "prototypes",
    PackageInfo::ImplicitPackageFamily{"families/", {}, {}}
  );

  EXPECT_EQ(
    info.mangleForCacheKey(),
    R"([{"implicit_families":{"enable_strict_isolation":false,"include_paths":[],"includes":[],"soft_includes":[]}},{"prototypes":{"includes":[],"path":"families/","soft_includes":[]}}])"
  );
}

// Checks that changing any implicit package family field changes the cache key.
TEST(PackageInfoTest, EveryImplicitFamilyFieldChangesCacheMangle) {
  PackageInfo base;
  PackageInfo::ImplicitPackageFamily family;
  family.m_path = "www/prototypes/";
  family.m_includes.emplace("intern");
  family.m_soft_includes.emplace("soft");
  base.m_implicitPackageFamilies.emplace(
    "prototypes",
    std::move(family)
  );

  auto pathChanged = base;
  pathChanged.m_implicitPackageFamilies.at("prototypes").m_path =
    "www/other/";

  auto includesChanged = base;
  includesChanged.m_implicitPackageFamilies.at("prototypes")
    .m_includes.emplace("other");

  auto softIncludesChanged = base;
  softIncludesChanged.m_implicitPackageFamilies.at("prototypes")
    .m_soft_includes.emplace("other-soft");

  auto const baseMangle = base.mangleForCacheKey();
  EXPECT_NE(baseMangle, pathChanged.mangleForCacheKey());
  EXPECT_NE(baseMangle, includesChanged.mangleForCacheKey());
  EXPECT_NE(baseMangle, softIncludesChanged.mangleForCacheKey());
}

PackageInfo implicitPackageInfo() {
  PackageInfo info;
  PackageInfo::ImplicitPackageFamily family;
  family.m_path = "www/prototypes/";
  info.m_implicitPackageFamilies.emplace("prototypes", std::move(family));
  return info;
}

// Checks that an implicit package name resolves to its path prefix.
TEST(PackageInfoTest, ResolvesImplicitPackageNamesToPathPrefixes) {
  auto const info = implicitPackageInfo();
  EXPECT_EQ(
    info.implicitPackageNameToPathPrefix("prototypes.alpha"),
    std::optional<std::string>{"www/prototypes/alpha/"}
  );
}

// Checks that names without a declared implicit family do not resolve.
TEST(PackageInfoTest, RejectsUnknownImplicitPackageNamesToPathPrefixes) {
  auto const info = implicitPackageInfo();
  for (auto const name : {
         "prototypes",
         ".alpha",
         "prototypes.",
         "unknown.alpha",
       }) {
    EXPECT_FALSE(info.implicitPackageNameToPathPrefix(name).has_value()) << name;
  }
}

} // namespace
} // namespace HPHP
