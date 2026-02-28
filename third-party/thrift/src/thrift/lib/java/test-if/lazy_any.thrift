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

include "thrift/conformance/if/any.thrift"
include "thrift/annotation/thrift.thrift"

package "test.dev/thrift/lib/java/test/any"

namespace java.swift com.facebook.thrift.test.any

struct Drawing {
  1: string name;
  2: list<any.LazyAny> shapes; // Circle or Rectangle
}

struct Position {
  1: i32 x;
  2: i32 y;
}

struct Circle {
  1: i32 color;
  2: i32 radius;
  4: Position position;
}

@thrift.Uri{value = "test.dev/thrift/lib/java/test/any/Rec"}
struct Rectangle {
  1: i32 color;
  2: i32 len;
  3: i32 width;
  4: Position position;
  5: any.LazyAny canvas; // can be Image or SolidColor
}

struct Image {
  1: binary jpg;
}

struct SolidColor {
  1: i32 color;
}
