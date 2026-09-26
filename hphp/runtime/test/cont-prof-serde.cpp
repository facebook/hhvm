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

#include "hphp/runtime/vm/jit/cont-prof-serde.h"

#include <cstddef>
#include <cstdint>

#include <gtest/gtest.h>

#include "hphp/util/sha1.h"

namespace HPHP::jit {
namespace {

ContProfFuncKey exampleKey() {
  ContProfFuncKey key{};
  key.resolutionUnitPath = "src/example.php";
  key.bytecodeUnitHash = SHA1{uint64_t{1}};
  key.functionName = "example";
  return key;
}

void expectFunctionKeyRoundTrip(const ContProfFuncKey& expected) {
  auto const serialized = serializeContProfFuncKey(expected);
  ASSERT_TRUE(serialized);

  auto const deserialized = deserializeContProfFuncKey(
    folly::ByteRange{serialized->data(), serialized->size()}
  );
  ASSERT_TRUE(deserialized);
  EXPECT_EQ(expected, *deserialized);
}

TEST(ContProfSerde, FunctionKeyRoundTrip) {
  expectFunctionKeyRoundTrip(exampleKey());

  auto key = exampleKey();
  key.bytecodeUnitPath = "src/bytecode.php";
  key.className = "ExampleClass";
  key.closureContextName = "ClosureContext";
  expectFunctionKeyRoundTrip(key);
}

TEST(ContProfSerde, RejectsInvalidFunctionKey) {
  auto key = exampleKey();
  key.bytecodeUnitHash = SHA1{};

  EXPECT_FALSE(serializeContProfFuncKey(key));
}

TEST(ContProfSerde, RejectsMalformedInput) {
  auto const serialized = serializeContProfFuncKey(exampleKey());
  ASSERT_TRUE(serialized);

  for (size_t size = 0; size < serialized->size(); ++size) {
    EXPECT_FALSE(deserializeContProfFuncKey(
      folly::ByteRange{serialized->data(), size}
    ));
  }

  auto trailingByte = *serialized;
  trailingByte.push_back(0);
  EXPECT_FALSE(deserializeContProfFuncKey(
    folly::ByteRange{trailingByte.data(), trailingByte.size()}
  ));

  auto wrongSchema = *serialized;
  auto const schemaOffset = sizeof(size_t);
  ASSERT_GT(wrongSchema.size(), schemaOffset);
  wrongSchema[schemaOffset] ^= 1;
  EXPECT_FALSE(deserializeContProfFuncKey(
    folly::ByteRange{wrongSchema.data(), wrongSchema.size()}
  ));
}

}
}
