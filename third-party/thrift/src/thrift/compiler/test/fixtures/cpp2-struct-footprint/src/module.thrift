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

include "thrift/annotation/cpp.thrift"

namespace cpp2 cpp2_struct_footprint

// Basic struct types
struct SimpleStruct {
  1: i32 field;
}

struct ComplexStruct {
  1: SimpleStruct nested;
}

@cpp.Type{name = "SimpleStruct"}
typedef i32 I32Alias

struct Struct1 {}
struct Struct2 {}
struct Struct3 {}
struct Struct4 {}
struct Struct5 {}

safe server exception ExStruct {
  1: string message;
}

// Union type
union TestUnion {
  1: i32 int_value;
  2: SimpleStruct struct_value;
}

// Typedefs to test typedef resolution
typedef i32 MyInt
typedef SimpleStruct MyStruct
typedef list<SimpleStruct> StructList
typedef map<string, SimpleStruct> StructMap
typedef set<SimpleStruct> StructSet

// Interaction for testing interaction constructors
interaction Calculator {
  i32 add(1: i32 a, 2: i32 b);
}

@cpp.Type{name = "folly::IOBuf"}
typedef binary IOBuf
@cpp.Type{name = "std::unique_ptr<folly::IOBuf>"}
typedef binary IOBufPtr

// Main service to test all code paths in extract_struct_footprint
service FootprintTestService {
  void processIOBuf(1: IOBuf buf, 2: IOBufPtr ptr, 3: I32Alias alias);
  // Test basic return type extraction
  SimpleStruct getStruct();

  // Test parameter extraction
  void setStruct(1: SimpleStruct input);

  // Test container type extraction in parameters
  void setStructList(1: list<SimpleStruct> items);

  // Test container type extraction in return type
  list<SimpleStruct> getStructList();

  // Test nested container extraction
  map<string, list<SimpleStruct>> getNestedContainer();

  // Test typedef resolution
  MyStruct getTypedefStruct();

  // Test typedef container resolution
  StructList getTypedefList();

  // Test union extraction
  TestUnion getUnion();

  // Test interaction constructor
  Calculator getCalculator();

  // Test stream element type extraction
  stream<SimpleStruct> streamStructs();

  // Test combined stream and sink
  //stream<i32> streamWithSink(1: sink<i32, string> input);
  Struct1, stream<SimpleStruct> streamWithSinkInitial(1: i32 input);

  Struct1, stream<SimpleStruct> streamWithSinkException(1: i32 input) throws (
    ExStruct ex,
  );
}

// Extended service to test inheritance
service ExtendedFootprintService extends FootprintTestService {
  // Additional method with complex types
  map<TestUnion, list<ComplexStruct>> getComplexMap();
}
