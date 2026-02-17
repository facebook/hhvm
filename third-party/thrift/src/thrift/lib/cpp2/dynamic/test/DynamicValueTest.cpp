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

#include <thrift/lib/cpp2/dynamic/DynamicValue.h>

#include <gtest/gtest.h>

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>
#include <thrift/lib/cpp2/dynamic/List.h>
#include <thrift/lib/cpp2/dynamic/Map.h>
#include <thrift/lib/cpp2/dynamic/Path.h>
#include <thrift/lib/cpp2/dynamic/Serialization.h>
#include <thrift/lib/cpp2/dynamic/Set.h>
#include <thrift/lib/cpp2/dynamic/Struct.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>
#include <thrift/lib/cpp2/dynamic/Union.h>
#include <thrift/lib/cpp2/dynamic/detail/ConcreteList.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>

#include <limits>

namespace apache::thrift::dynamic {
namespace {

// Primitive value creation and intrinsic defaults
TEST(DynamicValueTest, PrimitiveValues) {
  // Bool
  auto trueVal = DynamicValue::makeBool(true);
  auto falseVal = DynamicValue::makeBool(false);
  EXPECT_EQ(trueVal.type().kind(), type_system::TypeRef::Kind::BOOL);
  EXPECT_TRUE(trueVal.asBool());
  EXPECT_FALSE(falseVal.asBool());

  // Byte
  auto byteVal = DynamicValue::makeByte(42);
  EXPECT_EQ(byteVal.type().kind(), type_system::TypeRef::Kind::BYTE);
  EXPECT_EQ(byteVal.asByte(), 42);

  // I16
  auto i16Val = DynamicValue::makeI16(1000);
  EXPECT_EQ(i16Val.type().kind(), type_system::TypeRef::Kind::I16);
  EXPECT_EQ(i16Val.asI16(), 1000);

  // I32
  auto i32Val = DynamicValue::makeI32(100000);
  EXPECT_EQ(i32Val.type().kind(), type_system::TypeRef::Kind::I32);
  EXPECT_EQ(i32Val.asI32(), 100000);

  // I64
  auto i64Val = DynamicValue::makeI64(1000000000LL);
  EXPECT_EQ(i64Val.type().kind(), type_system::TypeRef::Kind::I64);
  EXPECT_EQ(i64Val.asI64(), 1000000000LL);

  // Float
  auto floatVal = DynamicValue::makeFloat(3.14f);
  EXPECT_EQ(floatVal.type().kind(), type_system::TypeRef::Kind::FLOAT);
  EXPECT_FLOAT_EQ(floatVal.asFloat(), 3.14f);

  // Double
  auto doubleVal = DynamicValue::makeDouble(3.14159265359);
  EXPECT_EQ(doubleVal.type().kind(), type_system::TypeRef::Kind::DOUBLE);
  EXPECT_DOUBLE_EQ(doubleVal.asDouble(), 3.14159265359);
}

TEST(DynamicValueTest, MakeDefault) {
  auto boolVal = DynamicValue::makeDefault(type_system::TypeSystem::Bool());
  EXPECT_FALSE(boolVal.asBool());

  auto i32Val = DynamicValue::makeDefault(type_system::TypeSystem::I32());
  EXPECT_EQ(i32Val.asI32(), 0);

  auto floatVal = DynamicValue::makeDefault(type_system::TypeSystem::Float());
  EXPECT_FLOAT_EQ(floatVal.asFloat(), 0.0f);
}

// Copy and move semantics
TEST(DynamicValueTest, CopyMoveSemantics) {
  auto orig = DynamicValue::makeI32(42);
  auto copied = orig;

  EXPECT_EQ(orig.asI32(), 42);
  EXPECT_EQ(copied.asI32(), 42);
  EXPECT_EQ(orig, copied);

  // Modify copy shouldn't affect original for primitives
  // (though primitives are immutable, this tests the copy works)
  auto copy2 = DynamicValue::makeI32(99);
  EXPECT_NE(orig, copy2);

  auto moved = std::move(orig);

  EXPECT_EQ(moved.asI32(), 42);
  // Note: orig is in moved-from state, don't access it
}

// Equality and comparison
TEST(DynamicValueTest, Equality) {
  auto val1 = DynamicValue::makeI32(42);
  auto val2 = DynamicValue::makeI32(42);
  auto val3 = DynamicValue::makeI32(99);

  EXPECT_TRUE(val1 == val2);
  EXPECT_FALSE(val1 == val3);
  EXPECT_TRUE(val1 != val3);

  // Different types are not equal
  auto val4 = DynamicValue::makeI64(42);
  EXPECT_FALSE(val1 == val4);
}

// Type checking
TEST(DynamicValueTest, TypeChecking) {
  auto value = DynamicValue::makeI32(42);

  EXPECT_TRUE(value.is<type_system::TypeRef::Kind::I32>());
  EXPECT_FALSE(value.is<type_system::TypeRef::Kind::I64>());
  EXPECT_FALSE(value.is<type_system::TypeRef::Kind::BOOL>());
}

TEST(DynamicValueTest, WrongTypeAccess) {
  auto value = DynamicValue::makeI32(42);

  EXPECT_THROW(value.asBool(), std::runtime_error);
  EXPECT_THROW(value.asByte(), std::runtime_error);
  EXPECT_THROW(value.asI16(), std::runtime_error);
  EXPECT_THROW(value.asI64(), std::runtime_error);
  EXPECT_THROW(value.asFloat(), std::runtime_error);
  EXPECT_THROW(value.asDouble(), std::runtime_error);
}

// Serialization round-trip
TEST(DynamicValueTest, SerializationRoundTrip) {
  folly::IOBufQueue bufQueue;
  CompactProtocolWriter writer;
  writer.setOutput(&bufQueue);

  // Serialize various values
  auto boolVal = DynamicValue::makeBool(true);
  auto byteVal = DynamicValue::makeByte(42);
  auto i16Val = DynamicValue::makeI16(1000);
  auto i32Val = DynamicValue::makeI32(100000);
  auto i64Val = DynamicValue::makeI64(1000000000LL);
  auto floatVal = DynamicValue::makeFloat(3.14f);
  auto doubleVal = DynamicValue::makeDouble(2.71828);

  serializeValue(writer, boolVal);
  serializeValue(writer, byteVal);
  serializeValue(writer, i16Val);
  serializeValue(writer, i32Val);
  serializeValue(writer, i64Val);
  serializeValue(writer, floatVal);
  serializeValue(writer, doubleVal);

  // Deserialize and verify
  auto buf = bufQueue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());

  auto deserBool = deserializeValue(reader, type_system::TypeSystem::Bool());
  auto deserByte = deserializeValue(reader, type_system::TypeSystem::Byte());
  auto deserI16 = deserializeValue(reader, type_system::TypeSystem::I16());
  auto deserI32 = deserializeValue(reader, type_system::TypeSystem::I32());
  auto deserI64 = deserializeValue(reader, type_system::TypeSystem::I64());
  auto deserFloat = deserializeValue(reader, type_system::TypeSystem::Float());
  auto deserDouble =
      deserializeValue(reader, type_system::TypeSystem::Double());

  EXPECT_EQ(boolVal, deserBool);
  EXPECT_EQ(byteVal, deserByte);
  EXPECT_EQ(i16Val, deserI16);
  EXPECT_EQ(i32Val, deserI32);
  EXPECT_EQ(i64Val, deserI64);
  EXPECT_EQ(floatVal, deserFloat);
  EXPECT_EQ(doubleVal, deserDouble);
}

// Serialization interoperability
TEST(DynamicValueTest, SerializationInteroperability) {
  // Test interoperability: write static types, read as DynamicValue
  folly::IOBufQueue bufQueue;
  CompactProtocolWriter writer;
  writer.setOutput(&bufQueue);

  // Write using native protocol methods (simulating static Thrift types)
  writer.writeBool(true);
  writer.writeByte(42);
  writer.writeI16(1000);
  writer.writeI32(100000);
  writer.writeI64(1000000000LL);
  writer.writeFloat(3.14f);
  writer.writeDouble(2.71828);

  // Read using DynamicValue
  auto buf = bufQueue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());

  auto boolVal = deserializeValue(reader, type_system::TypeSystem::Bool());
  auto byteVal = deserializeValue(reader, type_system::TypeSystem::Byte());
  auto i16Val = deserializeValue(reader, type_system::TypeSystem::I16());
  auto i32Val = deserializeValue(reader, type_system::TypeSystem::I32());
  auto i64Val = deserializeValue(reader, type_system::TypeSystem::I64());
  auto floatVal = deserializeValue(reader, type_system::TypeSystem::Float());
  auto doubleVal = deserializeValue(reader, type_system::TypeSystem::Double());

  EXPECT_TRUE(boolVal.asBool());
  EXPECT_EQ(byteVal.asByte(), 42);
  EXPECT_EQ(i16Val.asI16(), 1000);
  EXPECT_EQ(i32Val.asI32(), 100000);
  EXPECT_EQ(i64Val.asI64(), 1000000000LL);
  EXPECT_FLOAT_EQ(floatVal.asFloat(), 3.14f);
  EXPECT_DOUBLE_EQ(doubleVal.asDouble(), 2.71828);

  // Test the reverse: write DynamicValue, read using native protocol methods
  folly::IOBufQueue bufQueue2;
  CompactProtocolWriter writer2;
  writer2.setOutput(&bufQueue2);

  serializeValue(writer2, DynamicValue::makeBool(false));
  serializeValue(writer2, DynamicValue::makeI32(-42));
  serializeValue(writer2, DynamicValue::makeDouble(1.234));

  auto buf2 = bufQueue2.move();
  CompactProtocolReader reader2;
  reader2.setInput(buf2.get());

  bool boolResult;
  int32_t i32Result;
  double doubleResult;

  reader2.readBool(boolResult);
  reader2.readI32(i32Result);
  reader2.readDouble(doubleResult);

  EXPECT_FALSE(boolResult);
  EXPECT_EQ(i32Result, -42);
  EXPECT_DOUBLE_EQ(doubleResult, 1.234);
}

// Serialization of special values
TEST(DynamicValueTest, SerializeSpecialValues) {
  folly::IOBufQueue bufQueue;
  CompactProtocolWriter writer;
  writer.setOutput(&bufQueue);

  auto inf = DynamicValue::makeDouble(std::numeric_limits<double>::infinity());
  auto negInf =
      DynamicValue::makeDouble(-std::numeric_limits<double>::infinity());
  auto nan = DynamicValue::makeDouble(std::numeric_limits<double>::quiet_NaN());

  serializeValue(writer, inf);
  serializeValue(writer, negInf);
  serializeValue(writer, nan);

  auto buf = bufQueue.move();
  CompactProtocolReader reader;
  reader.setInput(buf.get());

  auto deserInf = deserializeValue(reader, type_system::TypeSystem::Double());
  auto deserNegInf =
      deserializeValue(reader, type_system::TypeSystem::Double());
  auto deserNaN = deserializeValue(reader, type_system::TypeSystem::Double());

  EXPECT_TRUE(std::isinf(deserInf.asDouble()));
  EXPECT_TRUE(std::isinf(deserNegInf.asDouble()));
  EXPECT_TRUE(std::isnan(deserNaN.asDouble()));
}

// DynamicRef tests
TEST(DynamicRefTest, CreateFromValue) {
  auto value = DynamicValue::makeI32(42);
  DynamicRef ref(value);

  EXPECT_EQ(ref.type().kind(), type_system::TypeRef::Kind::I32);
  EXPECT_EQ(ref.asI32(), 42);
}

TEST(DynamicRefTest, MutableAccess) {
  auto value = DynamicValue::makeI32(42);
  DynamicRef ref(value);

  // Modify through reference
  ref.asI32() = 100;

  // Value should be modified
  EXPECT_EQ(value.asI32(), 100);
  EXPECT_EQ(ref.asI32(), 100);
}

TEST(DynamicRefTest, CopyMethod) {
  auto value = DynamicValue::makeI32(42);
  DynamicRef ref(value);

  auto copy = ref.copy();
  EXPECT_EQ(copy.asI32(), 42);
  EXPECT_EQ(copy, value);

  // Modify original through ref
  ref.asI32() = 100;

  // Copy should be independent
  EXPECT_EQ(copy.asI32(), 42);
  EXPECT_EQ(value.asI32(), 100);
}

TEST(DynamicRefTest, Equality) {
  auto val1 = DynamicValue::makeI32(42);
  auto val2 = DynamicValue::makeI32(42);
  auto val3 = DynamicValue::makeI32(99);

  DynamicRef ref1(val1);
  DynamicRef ref2(val2);
  DynamicRef ref3(val3);

  // Refs to same value should be equal
  EXPECT_TRUE(ref1 == ref2);
  EXPECT_FALSE(ref1 == ref3);

  // Ref should equal original value
  EXPECT_TRUE(ref1 == val1);
  EXPECT_TRUE(val1 == ref1);
}

TEST(DynamicRefTest, TypeCheckingAndAccess) {
  auto value = DynamicValue::makeI32(42);
  DynamicRef ref(value);

  // Type checking
  EXPECT_TRUE(ref.is<type_system::TypeRef::Kind::I32>());
  EXPECT_FALSE(ref.is<type_system::TypeRef::Kind::I64>());
  EXPECT_FALSE(ref.is<type_system::TypeRef::Kind::BOOL>());

  // Wrong type access throws
  EXPECT_THROW(ref.asBool(), std::runtime_error);
  EXPECT_THROW(ref.asByte(), std::runtime_error);
  EXPECT_THROW(ref.asI16(), std::runtime_error);
  EXPECT_THROW(ref.asI64(), std::runtime_error);
  EXPECT_THROW(ref.asFloat(), std::runtime_error);
  EXPECT_THROW(ref.asDouble(), std::runtime_error);
}

TEST(DynamicRefTest, AssignMethod) {
  auto val1 = DynamicValue::makeI32(42);
  auto val2 = DynamicValue::makeI32(100);

  DynamicRef ref1(val1);
  DynamicRef ref2(val2);

  // Assign val2's value to val1
  ref1.assign(ref2);

  // val1 should now have val2's value
  EXPECT_EQ(val1.asI32(), 100);
  EXPECT_EQ(ref1.asI32(), 100);
  EXPECT_EQ(val2.asI32(), 100);

  // Modify val1 through ref1
  ref1.asI32() = 200;

  // val2 should be unchanged
  EXPECT_EQ(val1.asI32(), 200);
  EXPECT_EQ(val2.asI32(), 100);

  // Test assign from DynamicValue&&
  auto val3 = DynamicValue::makeI32(300);
  ref1.assign(std::move(val3));

  // val1 should now have 300
  EXPECT_EQ(val1.asI32(), 300);
  EXPECT_EQ(ref1.asI32(), 300);
  // val3 is in moved-from state, don't access it
}

TEST(DynamicRefTest, DebugString) {
  auto value = DynamicValue::makeI32(42);
  DynamicRef ref(value);

  std::string debugStr = ref.debugString();
  EXPECT_FALSE(debugStr.empty());
}

// Helper to create a list type
inline type_system::TypeRef::List makeListType(
    type_system::TypeRef elementType) {
  static type_system::detail::ContainerTypeCache cache;
  return type_system::TypeRef::List::of(elementType, cache);
}

// Tests for DynamicConstRef equality with mixed Datum/concrete pointer cases
TEST(DynamicConstRefTest, EqualityDatumVsConcrete) {
  // Create a list to get concrete pointers
  auto list = makeList(makeListType(type_system::TypeSystem::I32()));
  list.push_back(DynamicValue::makeI32(42));
  list.push_back(DynamicValue::makeI32(99));

  // list[i] returns DynamicConstRef with concrete int32_t* pointer
  DynamicConstRef concreteRef1 = list[0];
  DynamicConstRef concreteRef2 = list[1];

  // Create DynamicValue which stores Datum internally
  auto val1 = DynamicValue::makeI32(42);
  auto val2 = DynamicValue::makeI32(99);

  // DynamicConstRef from DynamicValue has Datum* pointer
  DynamicConstRef datumRef1(val1);
  DynamicConstRef datumRef2(val2);

  // Datum vs Datum (both have Datum pointers)
  EXPECT_TRUE(datumRef1 == datumRef1);
  EXPECT_TRUE(datumRef1 == DynamicConstRef(DynamicValue::makeI32(42)));
  EXPECT_FALSE(datumRef1 == datumRef2);

  // Concrete vs Concrete (both have concrete pointers)
  EXPECT_TRUE(concreteRef1 == concreteRef1);
  EXPECT_FALSE(concreteRef1 == concreteRef2);

  // Datum vs Concrete (mixed pointer types, same values)
  EXPECT_TRUE(datumRef1 == concreteRef1);
  EXPECT_TRUE(concreteRef1 == datumRef1);
  EXPECT_FALSE(datumRef1 == concreteRef2);
  EXPECT_FALSE(concreteRef2 == datumRef1);

  // Datum vs Concrete (mixed pointer types, different values)
  EXPECT_TRUE(datumRef2 == concreteRef2);
  EXPECT_TRUE(concreteRef2 == datumRef2);
  EXPECT_FALSE(datumRef2 == concreteRef1);
  EXPECT_FALSE(concreteRef1 == datumRef2);
}

TEST(DynamicConstRefTest, EqualityTypeMismatch) {
  // Create values of different types
  auto i32Val = DynamicValue::makeI32(42);
  auto i64Val = DynamicValue::makeI64(42);
  auto boolVal = DynamicValue::makeBool(true);
  auto floatVal = DynamicValue::makeFloat(42.0f);

  DynamicConstRef i32Ref(i32Val);
  DynamicConstRef i64Ref(i64Val);
  DynamicConstRef boolRef(boolVal);
  DynamicConstRef floatRef(floatVal);

  // Different types should not be equal even if numeric value is same
  EXPECT_FALSE(i32Ref == i64Ref);
  EXPECT_FALSE(i64Ref == i32Ref);
  EXPECT_FALSE(i32Ref == floatRef);
  EXPECT_FALSE(floatRef == i32Ref);
  EXPECT_FALSE(boolRef == i32Ref);
  EXPECT_FALSE(i32Ref == boolRef);
}

TEST(DynamicConstRefTest, EqualityTypeMismatchWithConcretePointers) {
  // Create lists of different element types to get concrete pointers
  auto i32List = makeList(makeListType(type_system::TypeSystem::I32()));
  i32List.push_back(DynamicValue::makeI32(42));

  auto i64List = makeList(makeListType(type_system::TypeSystem::I64()));
  i64List.push_back(DynamicValue::makeI64(42));

  auto boolList = makeList(makeListType(type_system::TypeSystem::Bool()));
  boolList.push_back(DynamicValue::makeBool(true));

  // Get concrete pointer refs
  DynamicConstRef i32ConcreteRef = i32List[0];
  DynamicConstRef i64ConcreteRef = i64List[0];
  DynamicConstRef boolConcreteRef = boolList[0];

  // Different types with concrete pointers should not be equal
  EXPECT_FALSE(i32ConcreteRef == i64ConcreteRef);
  EXPECT_FALSE(i64ConcreteRef == i32ConcreteRef);
  EXPECT_FALSE(boolConcreteRef == i32ConcreteRef);
  EXPECT_FALSE(i32ConcreteRef == boolConcreteRef);

  // Also test mixed Datum vs Concrete with type mismatch
  auto i64Val = DynamicValue::makeI64(42);
  DynamicConstRef i64DatumRef(i64Val);

  // Datum (i64) vs Concrete (i32) - type mismatch
  EXPECT_FALSE(i64DatumRef == i32ConcreteRef);
  EXPECT_FALSE(i32ConcreteRef == i64DatumRef);
}

TEST(DynamicConstRefTest, EqualityRefVsConstRef) {
  auto val1 = DynamicValue::makeI32(42);
  auto val2 = DynamicValue::makeI32(42);
  auto val3 = DynamicValue::makeI32(99);

  DynamicRef ref1(val1);
  DynamicConstRef constRef2(val2);
  DynamicConstRef constRef3(val3);

  // DynamicRef converts to DynamicConstRef for comparison
  EXPECT_TRUE(ref1 == constRef2);
  EXPECT_TRUE(constRef2 == ref1);
  EXPECT_FALSE(ref1 == constRef3);
  EXPECT_FALSE(constRef3 == ref1);

  // Also test with concrete pointers from list
  auto list = makeList(makeListType(type_system::TypeSystem::I32()));
  list.push_back(DynamicValue::makeI32(42));
  list.push_back(DynamicValue::makeI32(99));

  DynamicConstRef concreteRef1 = list[0];
  DynamicConstRef concreteRef2 = list[1];

  // DynamicRef (Datum) vs DynamicConstRef (concrete pointer)
  EXPECT_TRUE(ref1 == concreteRef1);
  EXPECT_TRUE(concreteRef1 == ref1);
  EXPECT_FALSE(ref1 == concreteRef2);
  EXPECT_FALSE(concreteRef2 == ref1);
}

// ============================================================================
// Traverse tests
// ============================================================================

using def = type_system::TypeSystemBuilder::DefinitionHelper;
using type_system::TypeIds;

// Helper fixture for traverse tests
struct TraverseTest : ::testing::Test {
  std::unique_ptr<type_system::TypeSystem> typeSystem;

  static constexpr auto kInnerUri = "facebook.com/thrift/test/Inner";
  static constexpr auto kOuterUri = "facebook.com/thrift/test/Outer";
  static constexpr auto kTestUnionUri = "facebook.com/thrift/test/TestUnion";

  TraverseTest() {
    type_system::TypeSystemBuilder builder;

    // struct Inner { 1: i32 value }
    builder.addType(
        kInnerUri,
        def::Struct({
            def::Field(
                def::Identity(1, "value"), def::AlwaysPresent, TypeIds::I32),
        }));

    // struct Outer {
    //   1: Inner inner
    //   2: optional Inner optionalInner
    //   3: list<i32> numbers
    //   4: map<string, i32> scores
    //   5: set<string> tags
    // }
    builder.addType(
        kOuterUri,
        def::Struct({
            def::Field(
                def::Identity(1, "inner"),
                def::AlwaysPresent,
                TypeIds::uri(kInnerUri)),
            def::Field(
                def::Identity(2, "optionalInner"),
                def::Optional,
                TypeIds::uri(kInnerUri)),
            def::Field(
                def::Identity(3, "numbers"),
                def::AlwaysPresent,
                TypeIds::list(TypeIds::I32)),
            def::Field(
                def::Identity(4, "scores"),
                def::AlwaysPresent,
                TypeIds::map(TypeIds::String, TypeIds::I32)),
            def::Field(
                def::Identity(5, "tags"),
                def::AlwaysPresent,
                TypeIds::set(TypeIds::String)),
        }));

    // union TestUnion { 1: i32 intValue; 2: string strValue }
    builder.addType(
        kTestUnionUri,
        def::Union({
            def::Field(
                def::Identity(1, "intValue"), def::Optional, TypeIds::I32),
            def::Field(
                def::Identity(2, "strValue"), def::Optional, TypeIds::String),
        }));

    typeSystem = std::move(builder).build();
  }

  const type_system::StructNode& outerNode() {
    return typeSystem->getUserDefinedTypeOrThrow(kOuterUri).asStruct();
  }

  const type_system::UnionNode& unionNode() {
    return typeSystem->getUserDefinedTypeOrThrow(kTestUnionUri).asUnion();
  }
};

TEST_F(TraverseTest, TraverseStructFields) {
  // Create an Outer struct with nested inner.value = 42
  auto outer = DynamicValue::makeDefault(outerNode().asRef());
  outer.asStruct().getField("inner")->asStruct().setField(
      "value", DynamicValue::makeI32(42));

  // Build path to inner.value
  PathBuilder pathBuilder{outerNode().asRef()};
  auto g1 = pathBuilder.enterField("inner");
  auto g2 = pathBuilder.enterField("value");
  Path path = pathBuilder.path();

  // Test const traverse via DynamicConstRef
  ASSERT_TRUE(DynamicConstRef(outer).traverse(path).has_value());
  EXPECT_EQ(DynamicConstRef(outer).traverse(path)->asI32(), 42);

  // Test mutable traverse via DynamicRef - modify and verify
  auto result = DynamicRef(outer).traverse(path);
  ASSERT_TRUE(result.has_value());
  result->asI32() = 100;
  EXPECT_EQ(
      outer.asStruct().getField("inner")->asStruct().getField("value")->asI32(),
      100);

  // Test traverse directly on DynamicValue (const and mutable)
  outer.asStruct().getField("inner")->asStruct().setField(
      "value", DynamicValue::makeI32(200));
  const auto& constOuter = outer;
  EXPECT_EQ(constOuter.traverse(path)->asI32(), 200);
  outer.traverse(path)->asI32() = 300;
  EXPECT_EQ(
      outer.asStruct().getField("inner")->asStruct().getField("value")->asI32(),
      300);

  // Test unset optional field returns nullopt
  PathBuilder optPathBuilder{outerNode().asRef()};
  auto og1 = optPathBuilder.enterField("optionalInner");
  auto og2 = optPathBuilder.enterField("value");
  EXPECT_FALSE(
      DynamicConstRef(outer).traverse(optPathBuilder.path()).has_value());
}

TEST_F(TraverseTest, TraverseList) {
  auto outer = DynamicValue::makeDefault(outerNode().asRef());
  auto& numbers = outer.asStruct().getField("numbers")->asList();
  numbers.push_back(DynamicValue::makeI32(10));
  numbers.push_back(DynamicValue::makeI32(20));

  // Build path to numbers[1]
  PathBuilder pathBuilder{outerNode().asRef()};
  auto g1 = pathBuilder.enterField("numbers");
  auto g2 = pathBuilder.enterListElement(1);
  Path path = pathBuilder.path();

  // Valid index - use mutable DynamicValue::traverse and modify
  auto result = outer.traverse(path);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->asI32(), 20);
  result->asI32() = 200;
  EXPECT_EQ(numbers.at(1).asI32(), 200);

  // Out of bounds - use const DynamicValue::traverse
  PathBuilder oobPathBuilder{outerNode().asRef()};
  auto og1 = oobPathBuilder.enterField("numbers");
  auto og2 = oobPathBuilder.enterListElement(99);
  const auto& constOuter = outer;
  EXPECT_FALSE(constOuter.traverse(oobPathBuilder.path()).has_value());
}

TEST_F(TraverseTest, TraverseMap) {
  auto outer = DynamicValue::makeDefault(outerNode().asRef());
  outer.asStruct().getField("scores")->asMap().insert(
      DynamicValue::makeString("alice"), DynamicValue::makeI32(100));

  // Build path to scores["alice"]
  PathBuilder pathBuilder{outerNode().asRef()};
  auto g1 = pathBuilder.enterField("scores");
  auto g2 = pathBuilder.enterMapValue("alice");
  Path path = pathBuilder.path();

  // Existing key - use DynamicRef::traverse and modify
  auto result = DynamicRef(outer).traverse(path);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->asI32(), 100);
  result->asI32() = 999;
  EXPECT_EQ(
      outer.asStruct()
          .getField("scores")
          ->asMap()
          .get(DynamicValue::makeString("alice"))
          ->asI32(),
      999);

  // Missing key - use DynamicValue::traverse directly
  PathBuilder missingPathBuilder{outerNode().asRef()};
  auto mg1 = missingPathBuilder.enterField("scores");
  auto mg2 = missingPathBuilder.enterMapValue("missing");
  EXPECT_FALSE(outer.traverse(missingPathBuilder.path()).has_value());
}

TEST_F(TraverseTest, TraverseSet) {
  auto outer = DynamicValue::makeDefault(outerNode().asRef());
  outer.asStruct().getField("tags")->asSet().insert(
      DynamicValue::makeString("important"));

  // Build path to tags{"important"}
  PathBuilder pathBuilder{outerNode().asRef()};
  auto g1 = pathBuilder.enterField("tags");
  auto g2 = pathBuilder.enterSetElement("important");
  Path path = pathBuilder.path();

  // Existing element - use const DynamicValue::traverse
  // (set elements are immutable, so mutable access throws)
  const auto& constOuter = outer;
  auto result = constOuter.traverse(path);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->asString().view(), "important");

  // Missing element - also use const access
  PathBuilder missingPathBuilder{outerNode().asRef()};
  auto mg1 = missingPathBuilder.enterField("tags");
  auto mg2 = missingPathBuilder.enterSetElement("missing");
  EXPECT_FALSE(constOuter.traverse(missingPathBuilder.path()).has_value());
}

TEST_F(TraverseTest, TraverseUnion) {
  auto unionVal = DynamicValue::makeDefault(unionNode().asRef());
  unionVal.asUnion().setField("intValue", DynamicValue::makeI32(42));

  // Build path to intValue (active field) - use mutable DynamicValue::traverse
  PathBuilder activePathBuilder{unionNode().asRef()};
  auto ag1 = activePathBuilder.enterField("intValue");
  auto result = unionVal.traverse(activePathBuilder.path());
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->asI32(), 42);
  result->asI32() = 100;
  EXPECT_EQ(unionVal.asUnion().getField("intValue").asI32(), 100);

  // Build path to strValue (inactive field) - use const DynamicValue::traverse
  PathBuilder inactivePathBuilder{unionNode().asRef()};
  auto ig1 = inactivePathBuilder.enterField("strValue");
  const auto& constUnion = unionVal;
  EXPECT_FALSE(constUnion.traverse(inactivePathBuilder.path()).has_value());
}

TEST_F(TraverseTest, TraverseTypeMismatchThrows) {
  // Try to access a field on a list type - should throw during path building
  PathBuilder pathBuilder{outerNode().asRef()};
  auto g1 = pathBuilder.enterField("numbers");
  EXPECT_THROW(
      (void)pathBuilder.enterField("nonexistent"), InvalidPathAccessError);
}

TEST_F(TraverseTest, TraverseEmptyPath) {
  auto value = DynamicValue::makeI32(42);
  PathBuilder pathBuilder{type_system::TypeSystem::I32()};
  Path path = pathBuilder.path();

  // Use DynamicValue::traverse directly (both const and mutable)
  const auto& constValue = value;
  EXPECT_EQ(constValue.traverse(path)->asI32(), 42);
  value.traverse(path)->asI32() = 100;
  EXPECT_EQ(value.asI32(), 100);
}

} // namespace
} // namespace apache::thrift::dynamic
