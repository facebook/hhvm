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

include "thrift/annotation/compat.thrift"

package "test.dev/thrift/lib/java/test/common"

namespace java.swift com.facebook.thrift.test.common

enum TestEnum {
  ZERO = 0,
  ONE = 1,
  TWO = 2,
}

@compat.Enums{type = compat.EnumType.Legacy}
enum TestEnumLegacy {
  ONE = 1,
  TWO = 2,
  THREE = 3,
}

@compat.Enums{type = compat.EnumType.Open}
enum TestEnumOpen {
  ZERO = 0,
  ONE = 1,
  TWO = 2,
}
