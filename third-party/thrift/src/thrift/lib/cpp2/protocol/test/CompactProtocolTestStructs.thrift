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

package "apache.org/thrift/test"

struct NestedStruct {
  1: i64 elem;
}

struct OriginalStruct {
  1: bool f1; // true
  // 2: bool f2; // false
  3: bool f3; // false
  // 4: bool f4; // true
  // 5: bool f5; // false
  6: byte f6; // 50
  // 7: i32 f7; // 1100
  8: i16 f8; // 1200
  // Note: a gap of 40 uses the two-byte encoding.
  48: i32 f48; // 1300
  // 68: i32 f68; // 1400
  // 88: i32 f88; // 1500
  100: i64 f100; // 1600
  20000: double f20000; // 1.0
  // 20020: string f20020; // "abc"
  20030: NestedStruct f20030; // {0}
  // 20031: NestedStruct f20031; // {1}
  20032: string f20032; // "def"
  // 20033: string f20033; // "ghi"
  20034: list<i32> f20034; // [0]
// 20035: list<i32> f20035; // [1]
}

struct UpdatedStruct {
  1: bool f1; // true
  2: bool f2; // false
  3: bool f3; // false
  4: bool f4; // true
  5: bool f5; // false
  6: byte f6; // 50
  7: i32 f7; // 1100
  8: i16 f8; // 1200
  // Note: a gap of 40 uses the two-byte encoding.
  48: i32 f48; // 1300
  68: i32 f68; // 1400
  88: i32 f88; // 1500
  100: i64 f100; // 1600
  20000: double f20000; // 1.0
  20020: string f20020; // "abc"
  20030: NestedStruct f20030; // {0}
  20031: NestedStruct f20031; // {1}
  20032: string f20032; // "def"
  20033: string f20033; // "ghi"
  20034: list<i32> f20034; // "[0]"
  20035: list<i32> f20035; // [1]
}
