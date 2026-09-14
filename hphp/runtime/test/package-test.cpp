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

namespace HPHP {
namespace {

// Checks that PackageInfo exposes implicit families only when enabled.
TEST(PackageInfoTest, LoadsImplicitPackageFamiliesOnlyWhenEnabled) {
  folly::test::TemporaryDirectory temp{"package-info"};
  auto const config = temp.path() / "PACKAGES.toml";
  std::ofstream out{config};
  out << R"(
[packages]
[packages.intern]
[packages.soft]

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
}

// Checks that the cache key includes an empty implicit package family map.
TEST(PackageInfoTest, MangleIncludesEmptyImplicitFamilies) {
  PackageInfo info;
  info.m_packages.emplace("example", PackageInfo::Package{});

  EXPECT_EQ(
    info.mangleForCacheKey(),
    R"([{"example":{"include_paths":[],"includes":[],"soft_includes":[]}},{}])"
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
    R"([{"implicit_families":{"include_paths":[],"includes":[],"soft_includes":[]}},{"prototypes":{"includes":[],"path":"families/","soft_includes":[]}}])"
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

} // namespace
} // namespace HPHP
