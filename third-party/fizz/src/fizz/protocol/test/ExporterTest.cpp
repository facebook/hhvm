/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/protocol/Exporter.h>
#include <fizz/protocol/OpenSSLFactory.h>

using namespace folly;

namespace fizz {
namespace test {

StringPiece exporter_master = {"12345678901234567890123456789012"};
StringPiece label = {"EXPORTER-Token-Binding"};
StringPiece basic_expected_ekm = {
    "55549d2d280d8507823a80c2ee69530e5dcc6a04e86f1bae1ef23a86337341a8"};

TEST(ExporterTest, TestExporterBasic) {
  OpenSSLFactory factory;
  auto ekm = Exporter::getExportedKeyingMaterial(
      factory,
      CipherSuite::TLS_AES_128_GCM_SHA256,
      folly::Range<const char*>(exporter_master),
      label,
      nullptr,
      32);

  EXPECT_EQ(StringPiece(ekm->coalesce()), unhexlify(basic_expected_ekm));
}
} // namespace test
} // namespace fizz
