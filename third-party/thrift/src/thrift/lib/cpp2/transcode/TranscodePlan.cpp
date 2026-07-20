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

#include <folly/container/F14Map.h>

#include <fmt/core.h>

#include <algorithm>
#include <utility>

namespace apache::thrift::transcode {

namespace {

int signedIntegerRank(ValueKind kind) {
  switch (kind) {
    case ValueKind::I8:
      return 1;
    case ValueKind::I16:
      return 2;
    case ValueKind::I32:
      return 3;
    case ValueKind::I64:
      return 4;
    case ValueKind::Bool:
    case ValueKind::Enum:
    case ValueKind::F32:
    case ValueKind::F64:
    case ValueKind::Bytes:
      return 0;
  }
}

folly::Expected<CoerceOp, CompileError> inferCoercion(
    ValueKind srcKind, ValueKind tgtKind) {
  if (srcKind == tgtKind) {
    return CoerceOp::None;
  }
  if (srcKind == ValueKind::Enum || tgtKind == ValueKind::Enum) {
    return folly::makeUnexpected(
        CompileError{fmt::format(
            "incompatible value kinds: {} → {}",
            static_cast<int>(srcKind),
            static_cast<int>(tgtKind))});
  }
  if (srcKind == ValueKind::F32 && tgtKind == ValueKind::F64) {
    return CoerceOp::WidenF32ToF64;
  }
  const int srcIntRank = signedIntegerRank(srcKind);
  const int tgtIntRank = signedIntegerRank(tgtKind);
  if (srcIntRank != 0 && srcIntRank < tgtIntRank) {
    return CoerceOp::WidenSignedInt;
  }
  return folly::makeUnexpected(
      CompileError{fmt::format(
          "incompatible value kinds: {} → {}",
          static_cast<int>(srcKind),
          static_cast<int>(tgtKind))});
}

// Recursively fuse two Commands
folly::Expected<Command, CompileError> fuseCommands(
    const Command& source, const Command& target);

} // namespace

folly::Expected<ScalarOp, CompileError> fuseScalarOps(
    const ScalarOp& source, const ScalarOp& target) {
  auto coercion = inferCoercion(source.valueKind, target.valueKind);
  if (coercion.hasError()) {
    return folly::makeUnexpected(coercion.error());
  }

  ScalarOp fused;
  fused.valueKind = target.valueKind;
  fused.readFn = source.readFn;
  fused.writeFn = target.writeFn;
  fused.coerce = *coercion;
  fused.customReadFn = source.customReadFn;
  fused.customWriteFn = target.customWriteFn;
  fused.memberOffset = target.memberOffset;
  fused.issetOffset = target.issetOffset;
  fused.typeInfoSetFn = target.typeInfoSetFn;
  fused.enumNames =
      target.enumNames != nullptr ? target.enumNames : source.enumNames;
  return fused;
}

folly::Expected<Command, CompileError> fuseStructOps(
    const StructOp& source, const StructOp& target) {
  // Index target fields by ID
  folly::F14FastMap<int16_t, const FieldEntry*> targetById;
  for (const auto& field : target.fields) {
    targetById[field.fieldId] = &field;
  }

  StructOp fused;
  fused.fieldIdent = source.fieldIdent;
  fused.writeFieldIdent = target.writeFieldIdent;
  fused.readFieldProto = source.readFieldProto;
  fused.writeFieldProto = target.writeFieldProto;
  fused.readFieldHeader = source.readFieldHeader;
  fused.writeFieldHeader = target.writeFieldHeader;
  fused.readEnd = source.readEnd;
  fused.writeEnd = target.writeEnd;
  fused.skipField = source.skipField;
  fused.readLengthDelimited = source.readLengthDelimited;
  fused.writeLengthDelimited = target.writeLengthDelimited;
  fused.schemaType =
      source.schemaType != nullptr ? source.schemaType : target.schemaType;
  fused.setIssetFn = target.setIssetFn;
  fused.getBasePtrFn = target.getBasePtrFn;

  for (const auto& srcField : source.fields) {
    auto it = targetById.find(srcField.fieldId);
    if (it == targetById.end()) {
      continue; // source field not in target — skipped at runtime
    }
    const auto& tgtField = *it->second;

    if (srcField.command == nullptr) {
      return folly::makeUnexpected(
          CompileError{fmt::format(
              "field {} (id={}) has no source command",
              srcField.fieldName,
              srcField.fieldId)});
    }
    if (tgtField.command == nullptr) {
      return folly::makeUnexpected(
          CompileError{fmt::format(
              "field {} (id={}) has no target command",
              srcField.fieldName,
              srcField.fieldId)});
    }

    // Handle protobuf unpacked repeated: source has element command,
    // target has SeqOp. Fuse the element commands directly.
    if (srcField.isRepeated && !tgtField.isRepeated &&
        std::holds_alternative<SeqOp>(*tgtField.command)) {
      const auto& tgtSeq = std::get<SeqOp>(*tgtField.command);
      if (tgtSeq.element == nullptr) {
        return folly::makeUnexpected(
            CompileError{fmt::format(
                "repeated field {} (id={}) has no target element command",
                srcField.fieldName,
                srcField.fieldId)});
      }
      auto fusedElem = fuseCommands(*srcField.command, *tgtSeq.element);
      if (fusedElem.hasError()) {
        return folly::makeUnexpected(
            CompileError{fmt::format(
                "repeated field {} (id={}): {}",
                srcField.fieldName,
                srcField.fieldId,
                fusedElem.error().message)});
      }

      FieldEntry entry;
      entry.fieldId = srcField.fieldId;
      entry.fieldName = srcField.fieldName;
      entry.readTypeInfo = srcField.readTypeInfo;
      entry.writeTypeInfo = tgtField.writeTypeInfo;
      entry.isRepeated = true;
      entry.optional = tgtField.optional;
      entry.required = tgtField.required;
      entry.hasCustomDefault = tgtField.hasCustomDefault;
      // Store fused element command + target SeqOp's write framing info
      // The codegen will use the write framing to produce the container header.
      SeqOp fusedSeq;
      fusedSeq.readFraming =
          ContainerFraming::None; // protobuf: no read framing
      fusedSeq.writeFraming = tgtSeq.writeFraming;
      fusedSeq.readLoopKind = LoopKind::ByCount; // unused for repeated
      fusedSeq.writeLoopKind = tgtSeq.writeLoopKind;
      fusedSeq.readElemType = 0;
      fusedSeq.writeElemType = tgtSeq.writeElemType;
      fusedSeq.element = std::make_unique<Command>(std::move(*fusedElem));
      entry.command = std::make_unique<Command>(std::move(fusedSeq));
      fused.fields.push_back(std::move(entry));
      continue;
    }

    auto fusedCmd = fuseCommands(*srcField.command, *tgtField.command);
    if (fusedCmd.hasError()) {
      return folly::makeUnexpected(
          CompileError{fmt::format(
              "field {} (id={}): {}",
              srcField.fieldName,
              srcField.fieldId,
              fusedCmd.error().message)});
    }

    FieldEntry entry;
    entry.fieldId = srcField.fieldId;
    entry.fieldName = srcField.fieldName;
    entry.readTypeInfo = srcField.readTypeInfo; // from source codec
    entry.writeTypeInfo = tgtField.writeTypeInfo; // from target codec
    entry.isRepeated = srcField.isRepeated;
    entry.optional = tgtField.optional;
    entry.required = tgtField.required;
    entry.hasCustomDefault = tgtField.hasCustomDefault;
    entry.command = std::make_unique<Command>(std::move(*fusedCmd));
    fused.fields.push_back(std::move(entry));
  }

  // Sort by field ID for jump table generation
  std::sort(
      fused.fields.begin(),
      fused.fields.end(),
      [](const auto& a, const auto& b) { return a.fieldId < b.fieldId; });

  return Command{std::move(fused)};
}

folly::Expected<TranscodePlan, CompileError> fuseCodecs(
    const Codec& source, const Codec& target) {
  auto fused = fuseCommands(source.root, target.root);
  if (fused.hasError()) {
    return folly::makeUnexpected(fused.error());
  }

  TranscodePlan plan{
      source.name + "_to_" + target.name,
      std::move(*fused),
  };
  plan.sourceProtocol = source.protocol;
  plan.targetProtocol = target.protocol;
  return plan;
}

namespace {

folly::Expected<Command, CompileError> fuseCommands(
    const Command& source, const Command& target) {
  // Both scalars → fuse scalar ops
  if (std::holds_alternative<ScalarOp>(source) &&
      std::holds_alternative<ScalarOp>(target)) {
    auto result =
        fuseScalarOps(std::get<ScalarOp>(source), std::get<ScalarOp>(target));
    if (result.hasError()) {
      return folly::makeUnexpected(result.error());
    }
    return Command{std::move(*result)};
  }

  // Both structs → fuse struct ops
  if (std::holds_alternative<StructOp>(source) &&
      std::holds_alternative<StructOp>(target)) {
    return fuseStructOps(
        std::get<StructOp>(source), std::get<StructOp>(target));
  }

  // Both sequences → fuse element commands
  if (std::holds_alternative<SeqOp>(source) &&
      std::holds_alternative<SeqOp>(target)) {
    const auto& srcSeq = std::get<SeqOp>(source);
    const auto& tgtSeq = std::get<SeqOp>(target);
    if (srcSeq.element == nullptr) {
      return folly::makeUnexpected(
          CompileError{"source sequence has no element command"});
    }
    if (tgtSeq.element == nullptr) {
      return folly::makeUnexpected(
          CompileError{"target sequence has no element command"});
    }
    auto fusedElem = fuseCommands(*srcSeq.element, *tgtSeq.element);
    if (fusedElem.hasError()) {
      return folly::makeUnexpected(
          CompileError{
              fmt::format("seq element: {}", fusedElem.error().message)});
    }
    SeqOp fused;
    fused.readFraming = srcSeq.readFraming;
    fused.writeFraming = tgtSeq.writeFraming;
    fused.readLoopKind = srcSeq.readLoopKind;
    fused.writeLoopKind = tgtSeq.writeLoopKind;
    fused.readElemType = srcSeq.readElemType;
    fused.writeElemType = tgtSeq.writeElemType;
    fused.element = std::make_unique<Command>(std::move(*fusedElem));
    return Command{std::move(fused)};
  }

  // Both maps → fuse key and value commands
  if (std::holds_alternative<MapOp>(source) &&
      std::holds_alternative<MapOp>(target)) {
    const auto& srcMap = std::get<MapOp>(source);
    const auto& tgtMap = std::get<MapOp>(target);
    if (srcMap.key == nullptr) {
      return folly::makeUnexpected(
          CompileError{"source map has no key command"});
    }
    if (tgtMap.key == nullptr) {
      return folly::makeUnexpected(
          CompileError{"target map has no key command"});
    }
    if (srcMap.value == nullptr) {
      return folly::makeUnexpected(
          CompileError{"source map has no value command"});
    }
    if (tgtMap.value == nullptr) {
      return folly::makeUnexpected(
          CompileError{"target map has no value command"});
    }
    auto fusedKey = fuseCommands(*srcMap.key, *tgtMap.key);
    if (fusedKey.hasError()) {
      return folly::makeUnexpected(
          CompileError{fmt::format("map key: {}", fusedKey.error().message)});
    }
    auto fusedVal = fuseCommands(*srcMap.value, *tgtMap.value);
    if (fusedVal.hasError()) {
      return folly::makeUnexpected(
          CompileError{fmt::format("map value: {}", fusedVal.error().message)});
    }
    MapOp fused;
    fused.readFraming = srcMap.readFraming;
    fused.writeFraming = tgtMap.writeFraming;
    fused.readKeyType = srcMap.readKeyType;
    fused.readValueType = srcMap.readValueType;
    fused.writeKeyType = tgtMap.writeKeyType;
    fused.writeValueType = tgtMap.writeValueType;
    fused.readEntryIsSubmessage = srcMap.readEntryIsSubmessage;
    fused.writeEntryIsSubmessage = tgtMap.writeEntryIsSubmessage;
    fused.readKeyWireType = srcMap.readKeyWireType;
    fused.readValueWireType = srcMap.readValueWireType;
    fused.writeKeyWireType = tgtMap.writeKeyWireType;
    fused.writeValueWireType = tgtMap.writeValueWireType;
    fused.key = std::make_unique<Command>(std::move(*fusedKey));
    fused.value = std::make_unique<Command>(std::move(*fusedVal));
    return Command{std::move(fused)};
  }

  return folly::makeUnexpected(
      CompileError{"cannot fuse commands of different structural kinds"});
}

} // namespace

} // namespace apache::thrift::transcode
