/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <thrift/lib/cpp2/transcode/WireType.h>

#include <thrift/lib/cpp2/protocol/CompactProtocol.h>

#include <folly/portability/GTest.h>

// Pin the locally-mirrored Compact type nibbles in WireType.h to the canonical
// apache::thrift::detail::compact enum. WireType.h can't include the heavy
// CompactProtocol header, so this test carries that dependency and fails to
// compile the moment the mirror drifts from the protocol.

namespace apache::thrift::transcode {
namespace {

namespace canonical = apache::thrift::detail::compact;

static_assert(wire::kCompactStop == canonical::CT_STOP);
static_assert(wire::kCompactBooleanTrue == canonical::CT_BOOLEAN_TRUE);
static_assert(wire::kCompactBooleanFalse == canonical::CT_BOOLEAN_FALSE);
static_assert(wire::kCompactByte == canonical::CT_BYTE);
static_assert(wire::kCompactI16 == canonical::CT_I16);
static_assert(wire::kCompactI32 == canonical::CT_I32);
static_assert(wire::kCompactI64 == canonical::CT_I64);
static_assert(wire::kCompactDouble == canonical::CT_DOUBLE);
static_assert(wire::kCompactBinary == canonical::CT_BINARY);
static_assert(wire::kCompactList == canonical::CT_LIST);
static_assert(wire::kCompactSet == canonical::CT_SET);
static_assert(wire::kCompactMap == canonical::CT_MAP);
static_assert(wire::kCompactStruct == canonical::CT_STRUCT);
static_assert(wire::kCompactFloat == canonical::CT_FLOAT);

TEST(WireTypeGuardTest, CompactMirrorMatchesCanonical) {
  EXPECT_EQ(wire::kCompactStop, canonical::CT_STOP);
}

} // namespace
} // namespace apache::thrift::transcode
