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

import FBThrift
import FBThriftTestsTerse
import Foundation
import Testing

/// Tests for @thrift.TerseWrite generated from terse_test.thrift.
struct GeneratedTerseTests {
  @Test func terseDefaultsAreOmitted() {
    // A terse field equal to its intrinsic default is not written to the wire,
    // so an all-default value is strictly smaller than one with a terse field set.
    let allDefault = TerseFields()
    var withTerse = TerseFields()
    withTerse.terse_int = 5
    #expect(
      CompactSerializer.serialize(allDefault).count
        < CompactSerializer.serialize(withTerse).count)
  }

  @Test func terseRoundTrip() throws {
    var t = TerseFields()
    t.terse_int = 7
    t.terse_string = "hi"
    t.normal_int = 3
    t.terse_list = [1, 2, 3]
    let data = CompactSerializer.serialize(t)
    #expect(
      try CompactSerializer.deserialize(TerseFields.self, from: data) == t)
  }

  @Test func terseContainerOmittedWhenEmpty() throws {
    // An empty terse container equals its intrinsic default and is omitted;
    // a non-empty one is written and round-trips.
    var withList = TerseFields()
    withList.terse_list = [9]
    #expect(
      CompactSerializer.serialize(TerseFields()).count
        < CompactSerializer.serialize(withList).count)
    let data = BinarySerializer.serialize(withList)
    #expect(
      try BinarySerializer.deserialize(TerseFields.self, from: data).terse_list
        == [9])
  }

  @Test func terseMissingFillsIntrinsicDefault() throws {
    // All-default encode omits the terse fields; decode restores intrinsic
    // defaults (same as always-present fields).
    let data = CompactSerializer.serialize(TerseFields())
    let decoded = try CompactSerializer.deserialize(TerseFields.self, from: data)
    #expect(decoded.terse_int == 0)
    #expect(decoded.terse_string == "")
    #expect(decoded.normal_int == 0)
    #expect(decoded.terse_list == [])
  }
}
