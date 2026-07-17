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

#include <thrift/lib/cpp2/transcode/TranscodePlan.h>

#include <gtest/gtest.h>

namespace apache::thrift::transcode {
namespace {

// A Compact-style i32 scalar read (source side): zigzag varint on the wire.
ScalarOp compactI32Read() {
  ScalarOp op;
  op.valueKind = ValueKind::I32;
  op.readFn = ReadFn::ZigzagVarint;
  op.writeFn = WriteFn::ZigzagVarint;
  return op;
}

// A Binary-style i64 scalar write (target side): fixed 64-bit big-endian.
ScalarOp binaryI64Write() {
  ScalarOp op;
  op.valueKind = ValueKind::I64;
  op.readFn = ReadFn::Fixed64BE;
  op.writeFn = WriteFn::Fixed64BE;
  return op;
}

ScalarOp scalarOp(ValueKind kind) {
  ScalarOp op;
  op.valueKind = kind;
  op.readFn = ReadFn::ZigzagVarint;
  op.writeFn = WriteFn::ZigzagVarint;
  return op;
}

void expectScalarCoercion(
    ValueKind sourceKind, ValueKind targetKind, CoerceOp expected) {
  SCOPED_TRACE(static_cast<int>(sourceKind));
  SCOPED_TRACE(static_cast<int>(targetKind));

  auto fused = fuseScalarOps(scalarOp(sourceKind), scalarOp(targetKind));
  ASSERT_FALSE(fused.hasError()) << fused.error().message;

  EXPECT_EQ(fused->valueKind, targetKind);
  EXPECT_EQ(fused->coerce, expected);
}

std::shared_ptr<const EnumNames> colorEnumNames() {
  auto names = std::make_shared<EnumNames>();
  names->values = {{1, "RED"}, {2, "BLUE"}};
  return names;
}

TEST(PlanFuseTest, FuseScalarOps_TakesReadFromSourceWriteFromTarget) {
  ScalarOp source = compactI32Read();
  ScalarOp target = binaryI64Write();

  auto fused = fuseScalarOps(source, target);
  ASSERT_FALSE(fused.hasError()) << fused.error().message;

  // Read side comes from the source codec, write side from the target codec,
  // and the value kind follows the target. i32 -> i64 widens.
  EXPECT_EQ(fused->readFn, ReadFn::ZigzagVarint);
  EXPECT_EQ(fused->writeFn, WriteFn::Fixed64BE);
  EXPECT_EQ(fused->valueKind, ValueKind::I64);
  EXPECT_EQ(fused->coerce, CoerceOp::WidenSignedInt);
}

TEST(PlanFuseTest, FuseScalarOps_SameKind_NoCoercion) {
  ScalarOp source = compactI32Read();

  ScalarOp target;
  target.valueKind = ValueKind::I32;
  target.readFn = ReadFn::Fixed32BE;
  target.writeFn = WriteFn::Fixed32BE;

  auto fused = fuseScalarOps(source, target);
  ASSERT_FALSE(fused.hasError()) << fused.error().message;

  EXPECT_EQ(fused->readFn, ReadFn::ZigzagVarint);
  EXPECT_EQ(fused->writeFn, WriteFn::Fixed32BE);
  EXPECT_EQ(fused->valueKind, ValueKind::I32);
  EXPECT_EQ(fused->coerce, CoerceOp::None);
}

TEST(PlanFuseTest, FuseScalarOps_SignedIntegerWideningUsesSingleCoercion) {
  expectScalarCoercion(ValueKind::I8, ValueKind::I16, CoerceOp::WidenSignedInt);
  expectScalarCoercion(ValueKind::I8, ValueKind::I32, CoerceOp::WidenSignedInt);
  expectScalarCoercion(ValueKind::I8, ValueKind::I64, CoerceOp::WidenSignedInt);
  expectScalarCoercion(
      ValueKind::I16, ValueKind::I32, CoerceOp::WidenSignedInt);
  expectScalarCoercion(
      ValueKind::I16, ValueKind::I64, CoerceOp::WidenSignedInt);
  expectScalarCoercion(
      ValueKind::I32, ValueKind::I64, CoerceOp::WidenSignedInt);
}

TEST(PlanFuseTest, FuseScalarOps_SignedIntegerNarrowingErrors) {
  auto fused =
      fuseScalarOps(scalarOp(ValueKind::I64), scalarOp(ValueKind::I32));
  EXPECT_TRUE(fused.hasError());
}

TEST(PlanFuseTest, FuseScalarOps_EnumKeepsLogicalKind) {
  auto sourceEnumNames = colorEnumNames();
  ScalarOp source;
  source.valueKind = ValueKind::Enum;
  source.readFn = ReadFn::ZigzagVarint;
  source.writeFn = WriteFn::ZigzagVarint;
  source.enumNames = sourceEnumNames;

  auto targetEnumNames = colorEnumNames();
  ScalarOp target;
  target.valueKind = ValueKind::Enum;
  target.readFn = ReadFn::ParseEnumNameOrDecimalText;
  target.writeFn = WriteFn::EnumNameOrDecimalText;
  target.enumNames = targetEnumNames;

  auto fused = fuseScalarOps(source, target);
  ASSERT_FALSE(fused.hasError()) << fused.error().message;

  EXPECT_EQ(fused->valueKind, ValueKind::Enum);
  EXPECT_EQ(fused->readFn, ReadFn::ZigzagVarint);
  EXPECT_EQ(fused->writeFn, WriteFn::EnumNameOrDecimalText);
  EXPECT_EQ(fused->coerce, CoerceOp::None);
  EXPECT_EQ(fused->enumNames, targetEnumNames);
}

TEST(PlanFuseTest, FuseScalarOps_EnumAndIntegerAreIncompatible) {
  ScalarOp source;
  source.valueKind = ValueKind::Enum;
  source.readFn = ReadFn::ZigzagVarint;
  source.writeFn = WriteFn::ZigzagVarint;
  source.enumNames = colorEnumNames();

  ScalarOp target;
  target.valueKind = ValueKind::I32;
  target.readFn = ReadFn::Fixed32BE;
  target.writeFn = WriteFn::Fixed32BE;

  auto fused = fuseScalarOps(source, target);
  EXPECT_TRUE(fused.hasError());
}

TEST(PlanFuseTest, FuseScalarOps_IncompatibleKinds_Errors) {
  ScalarOp source;
  source.valueKind = ValueKind::Bytes;
  source.readFn = ReadFn::LengthPrefixedVarint;
  source.writeFn = WriteFn::LengthPrefixedVarint;

  ScalarOp target = binaryI64Write();

  auto fused = fuseScalarOps(source, target);
  EXPECT_TRUE(fused.hasError());
}

// Build a single-field struct op. The field carries `fieldId` and an i32 scalar
// command built from (readFn, writeFn, valueKind).
FieldEntry scalarField(
    int16_t fieldId,
    std::string name,
    ValueKind kind,
    ReadFn readFn,
    WriteFn writeFn,
    uint8_t writeTypeInfo) {
  ScalarOp scalar;
  scalar.valueKind = kind;
  scalar.readFn = readFn;
  scalar.writeFn = writeFn;

  FieldEntry entry;
  entry.fieldId = fieldId;
  entry.fieldName = std::move(name);
  entry.writeTypeInfo = writeTypeInfo;
  entry.command = std::make_unique<Command>(std::move(scalar));
  return entry;
}

TEST(PlanFuseTest, FuseStructOps_MergesFramingAndPerFieldOps) {
  // Source: Compact-style read framing over two i32 fields.
  StructOp source;
  source.fieldIdent = FieldIdent::ById;
  source.readFieldHeader = "compact_read_field_header";
  source.writeFieldHeader = "compact_write_field_header";
  source.readEnd = "";
  source.writeEnd = "compact_write_stop";
  source.skipField = "compact_skip_field";
  source.fields.push_back(scalarField(
      1, "a", ValueKind::I32, ReadFn::ZigzagVarint, WriteFn::ZigzagVarint, 5));
  source.fields.push_back(scalarField(
      2, "b", ValueKind::I32, ReadFn::ZigzagVarint, WriteFn::ZigzagVarint, 5));

  // Target: Binary-style write framing. Field 1 stays i32, field 2 widens to
  // i64 so we can observe both value-kind pass-through and coercion.
  StructOp target;
  target.fieldIdent = FieldIdent::ById;
  target.writeFieldIdent = FieldIdent::ById;
  target.readFieldHeader = "binary_read_field_header";
  target.writeFieldHeader = "binary_write_field_header";
  target.readEnd = "";
  target.writeEnd = "binary_write_stop";
  target.skipField = "binary_skip_field";
  target.fields.push_back(scalarField(
      1, "a", ValueKind::I32, ReadFn::Fixed32BE, WriteFn::Fixed32BE, 8));
  target.fields.push_back(scalarField(
      2, "b", ValueKind::I64, ReadFn::Fixed64BE, WriteFn::Fixed64BE, 10));

  auto result = fuseStructOps(source, target);
  ASSERT_FALSE(result.hasError()) << result.error().message;
  ASSERT_TRUE(std::holds_alternative<StructOp>(*result));
  const auto& fused = std::get<StructOp>(*result);

  // Read framing (header/skip/end) comes from the source; write framing and the
  // write-side field identity come from the target.
  EXPECT_EQ(fused.readFieldHeader, "compact_read_field_header");
  EXPECT_EQ(fused.skipField, "compact_skip_field");
  EXPECT_EQ(fused.writeFieldHeader, "binary_write_field_header");
  EXPECT_EQ(fused.writeEnd, "binary_write_stop");
  EXPECT_EQ(fused.writeFieldIdent, FieldIdent::ById);

  // Both matched fields survive, sorted by id.
  ASSERT_EQ(fused.fields.size(), 2u);
  EXPECT_EQ(fused.fields[0].fieldId, 1);
  EXPECT_EQ(fused.fields[1].fieldId, 2);

  // Field 1: read from source (zigzag), write from target (fixed32), no coerce.
  ASSERT_TRUE(std::holds_alternative<ScalarOp>(*fused.fields[0].command));
  const auto& f0 = std::get<ScalarOp>(*fused.fields[0].command);
  EXPECT_EQ(f0.readFn, ReadFn::ZigzagVarint);
  EXPECT_EQ(f0.writeFn, WriteFn::Fixed32BE);
  EXPECT_EQ(f0.valueKind, ValueKind::I32);
  EXPECT_EQ(f0.coerce, CoerceOp::None);
  // writeTypeInfo is taken from the target codec.
  EXPECT_EQ(fused.fields[0].writeTypeInfo, 8);

  // Field 2: read from source (zigzag i32), write from target (fixed64 i64),
  // widening coercion inferred.
  ASSERT_TRUE(std::holds_alternative<ScalarOp>(*fused.fields[1].command));
  const auto& f1 = std::get<ScalarOp>(*fused.fields[1].command);
  EXPECT_EQ(f1.readFn, ReadFn::ZigzagVarint);
  EXPECT_EQ(f1.writeFn, WriteFn::Fixed64BE);
  EXPECT_EQ(f1.valueKind, ValueKind::I64);
  EXPECT_EQ(f1.coerce, CoerceOp::WidenSignedInt);
  EXPECT_EQ(fused.fields[1].writeTypeInfo, 10);
}

TEST(PlanFuseTest, FuseStructOps_TakesFieldWriteMetadataFromTarget) {
  StructOp source;
  source.fieldIdent = FieldIdent::ById;
  auto sourceField = scalarField(
      1, "a", ValueKind::I32, ReadFn::ZigzagVarint, WriteFn::ZigzagVarint, 5);
  sourceField.optional = true;
  sourceField.hasCustomDefault = true;
  source.fields.push_back(std::move(sourceField));

  StructOp target;
  target.fieldIdent = FieldIdent::ById;
  auto targetField = scalarField(
      1, "a", ValueKind::I32, ReadFn::Fixed32BE, WriteFn::Fixed32BE, 8);
  targetField.required = true;
  target.fields.push_back(std::move(targetField));

  auto result = fuseStructOps(source, target);
  ASSERT_FALSE(result.hasError()) << result.error().message;
  const auto& fused = std::get<StructOp>(*result);

  ASSERT_EQ(fused.fields.size(), 1u);
  EXPECT_FALSE(fused.fields[0].optional);
  EXPECT_TRUE(fused.fields[0].required);
  EXPECT_FALSE(fused.fields[0].hasCustomDefault);
}

TEST(PlanFuseTest, FuseStructOps_PreservesSourceRepeatedInGeneralPath) {
  StructOp source;
  source.fieldIdent = FieldIdent::ById;
  auto sourceField = scalarField(
      1, "a", ValueKind::I32, ReadFn::ZigzagVarint, WriteFn::ZigzagVarint, 5);
  sourceField.isRepeated = true;
  source.fields.push_back(std::move(sourceField));

  StructOp target;
  target.fieldIdent = FieldIdent::ById;
  target.fields.push_back(scalarField(
      1, "a", ValueKind::I32, ReadFn::Fixed32BE, WriteFn::Fixed32BE, 8));

  auto result = fuseStructOps(source, target);
  ASSERT_FALSE(result.hasError()) << result.error().message;
  const auto& fused = std::get<StructOp>(*result);

  ASSERT_EQ(fused.fields.size(), 1u);
  EXPECT_TRUE(fused.fields[0].isRepeated);
}

TEST(PlanFuseTest, FuseStructOps_NullFieldCommandErrors) {
  StructOp source;
  source.fieldIdent = FieldIdent::ById;
  FieldEntry sourceField;
  sourceField.fieldId = 1;
  sourceField.fieldName = "a";
  source.fields.push_back(std::move(sourceField));

  StructOp target;
  target.fieldIdent = FieldIdent::ById;
  target.fields.push_back(scalarField(
      1, "a", ValueKind::I32, ReadFn::Fixed32BE, WriteFn::Fixed32BE, 8));

  auto result = fuseStructOps(source, target);
  EXPECT_TRUE(result.hasError());
}

TEST(PlanFuseTest, FuseStructOps_SourceOnlyFieldDropped) {
  // Source has fields 1 and 2; target only has field 1. Field 2 is source-only
  // and must be dropped (skipped at runtime), never appearing in the fusion.
  StructOp source;
  source.fieldIdent = FieldIdent::ById;
  source.fields.push_back(scalarField(
      1, "a", ValueKind::I32, ReadFn::ZigzagVarint, WriteFn::ZigzagVarint, 5));
  source.fields.push_back(scalarField(
      2, "b", ValueKind::I32, ReadFn::ZigzagVarint, WriteFn::ZigzagVarint, 5));

  StructOp target;
  target.fieldIdent = FieldIdent::ById;
  target.fields.push_back(scalarField(
      1, "a", ValueKind::I32, ReadFn::Fixed32BE, WriteFn::Fixed32BE, 8));

  auto result = fuseStructOps(source, target);
  ASSERT_FALSE(result.hasError()) << result.error().message;
  const auto& fused = std::get<StructOp>(*result);

  ASSERT_EQ(fused.fields.size(), 1u);
  EXPECT_EQ(fused.fields[0].fieldId, 1);
}

} // namespace
} // namespace apache::thrift::transcode
