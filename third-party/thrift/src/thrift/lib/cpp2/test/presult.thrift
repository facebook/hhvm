/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

cpp_include "<deque>"

typedef binary (cpp2.type = "folly::IOBuf") IOBuf
typedef binary (cpp2.type = "std::unique_ptr<folly::IOBuf>") IOBufPtr
typedef list<i32> (cpp2.template = "std::deque") deque
typedef map<i32, i64> (cpp2.template = "std::unordered_map") unordered_map
typedef set<i32> (cpp2.template = "std::unordered_set") unordered_set

struct Struct {
  1: i32 x;
}

struct Noncopyable {
  1: IOBuf x;
}

exception Exception1 {
  1: i32 code;
}

exception Exception2 {
  2: string message;
}

enum Enum {
  Value1 = 0,
  Value2 = 1,
}

service PresultService {
  void methodVoid();
  bool methodBool(1: bool x);
  byte methodByte(1: byte x);
  i16 methodI16(1: i16 x);
  i32 methodI32(1: i32 x);
  i64 methodI64(1: i64 x);
  float methodFloat(1: float x);
  double methodDouble(1: double x);
  string methodString(1: string x);
  binary methodBinary(1: binary x);
  IOBuf methodIOBuf(1: IOBuf x);
  IOBufPtr methodIOBufPtr(1: IOBufPtr x);
  Enum methodEnum(1: Enum x);
  list<i32> methodList(1: list<i32> x);
  list<bool> methodListBool(1: list<bool> x);
  deque methodDeque(1: deque x);
  map<i32, i64> methodMap(1: map<i32, i64> x);
  unordered_map methodUnorderedMap(1: unordered_map x);
  set<i32> methodSet(1: set<i32> x);
  unordered_set methodUnorderedSet(1: unordered_set x);
  Struct methodStruct(1: Struct x);
  Struct methodException(1: i32 which) throws (
    1: Exception1 ex1,
    2: Exception2 ex2,
  );

  list<Noncopyable> methodNoncopyableList();
  set<Noncopyable> methodNoncopyableSet();
  map<i32, Noncopyable> methodNoncopyableMap();
}
