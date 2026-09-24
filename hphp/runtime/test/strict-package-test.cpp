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

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/vm/preclass-emitter.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/util/blob-encoder.h"

#include <gtest/gtest.h>

#include <memory>

namespace HPHP {
namespace {

std::unique_ptr<UnitEmitter> makeUnit(UnitEmitterAttributes attributes) {
  auto unit = std::make_unique<UnitEmitter>(
    SHA1{1}, SHA1{2}, RepoOptions::defaults().packageInfo(), attributes
  );
  unit->m_filepath = makeStaticString("C.php");
  auto const pce = unit->newPreClassEmitter("C");
  pce->init(
    1, 1, AttrNone, staticEmptyString(), staticEmptyString(), false
  );
  return unit;
}

TEST(StrictPackageTest, ProjectsStrictPackageOntoClasses) {
  auto strict = makeUnit(UnitEmitterAttributes{true, false});
  strict->finish();
  EXPECT_TRUE(strict->preclasses().front()->attrs() & AttrInStrictPackage);

  auto loose = makeUnit(UnitEmitterAttributes::defaults());
  loose->finish();
  EXPECT_FALSE(loose->preclasses().front()->attrs() & AttrInStrictPackage);
}

TEST(StrictPackageTest, ProjectsErrorPolicyOntoRuntimeUnit) {
  auto enforced = makeUnit(UnitEmitterAttributes{true, true});
  enforced->finish();
  EXPECT_TRUE(
    enforced->create()->shouldRaiseStrictPackageDynamicClassLoadError()
  );

  auto noticeOnly = makeUnit(UnitEmitterAttributes{true, false});
  noticeOnly->finish();
  EXPECT_FALSE(
    noticeOnly->create()->shouldRaiseStrictPackageDynamicClassLoadError()
  );
}

TEST(StrictPackageTest, PreservesProjectionsThroughSerialization) {
  auto unit = makeUnit(UnitEmitterAttributes{true, true});
  unit->finish();

  UnitEmitterSerdeWrapper encoded{std::move(unit)};
  BlobEncoder encoder;
  encoded.serde(encoder);

  UnitEmitterSerdeWrapper decoded;
  BlobDecoder decoder{encoder.data(), encoder.size()};
  decoded.serde(decoder);
  decoder.assertDone();

  ASSERT_NE(decoded.m_ue, nullptr);
  decoded.m_ue->finish();
  EXPECT_TRUE(
    decoded.m_ue->preclasses().front()->attrs() & AttrInStrictPackage
  );
  EXPECT_TRUE(
    decoded.m_ue->create()->shouldRaiseStrictPackageDynamicClassLoadError()
  );
}

TEST(StrictPackageTest, ComputesDiagnosticGateForDeployments) {
  PackageInfo packageInfo;
  packageInfo.m_packages.emplace("loose", PackageInfo::Package{});
  packageInfo.m_packages.emplace("strict", PackageInfo::Package{});
  packageInfo.m_packages.at("strict").m_enable_strict_isolation = true;
  packageInfo.m_implicitPackageFamilies.emplace(
    "family", PackageInfo::ImplicitPackageFamily{"families/", {}, {}}
  );
  packageInfo.m_deployments.emplace(
    "loose_only",
    PackageInfo::Deployment{{"loose"}, {}}
  );
  packageInfo.m_deployments.emplace(
    "strict_hard",
    PackageInfo::Deployment{{"strict"}, {}}
  );
  packageInfo.m_deployments.emplace(
    "strict_soft",
    PackageInfo::Deployment{{}, {"family.member"}}
  );

  EXPECT_FALSE(packageInfo.canReportStrictDynamicReference("loose_only"));
  EXPECT_FALSE(packageInfo.canReportStrictDynamicReference("missing"));
  EXPECT_TRUE(packageInfo.canReportStrictDynamicReference("strict_hard"));
  EXPECT_TRUE(packageInfo.canReportStrictDynamicReference("strict_soft"));
  EXPECT_TRUE(packageInfo.canReportStrictDynamicReference({}));
}

} // namespace
} // namespace HPHP
