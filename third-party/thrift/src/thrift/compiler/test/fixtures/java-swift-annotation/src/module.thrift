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

namespace java.swift test.fixtures.annotation

typedef map<i32, i64> (java.swift.type = "com.foo.FastIntLongMap") FMap

struct MyStruct {
  1: i64 intField;
  2: string stringField (java.swift.annotations = '@com.foo.Bar("BAZ")');
  3: string detailField (java.swift.annotations = "@com.foo.Ignored");
  4: FMap detailMap (java.swift.annotations = "@com.foo.Ignored");
  5: string titi (java.swift.name = "toto");
  6: string password (java.sensitive = "true");
} (
  java.swift.annotations = "@com.foo.Enabled @com.bar.AnotherAnnotation(val = 1)",
)

struct MyMutableStruct {
  1: i64 intField;
  2: string stringField;
} (java.swift.annotations = "@com.foo.Enabled", java.swift.mutable = "true")

struct MyMapping {
  1: map<i64, string> (java.swift.type = "com.foo.FastLongStringMap") lsMap;
  2: map<i32, FMap> (
    java.swift.type = "com.foo.FastIntObjectMap<com.foo.FastIntLongMap>",
  ) ioMap;
  3: map<string, string (java.swift.binary_string)> binaryMap;
  4: map<string, binary> regularBinary;
}
