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
import FBThriftTestsConstants
import Foundation
import Testing

/// Tests for constants generated from constants_test.thrift.
struct GeneratedConstantsTests {
  @Test func constantValues() {
    #expect(Constants.FLAG == true)
    #expect(Constants.OFFSET == -10)
    #expect(Constants.COUNT == 200)
    #expect(Constants.MASK == 0xFA12EE)
    #expect(Constants.BIG == 9000000000)
    #expect(abs(Constants.E - 2.718281828459) < 1e-12)
    #expect(Constants.DATE == "June 28, 2017")
    #expect(Constants.BLOB == Data("thrift".utf8))
  }
}
