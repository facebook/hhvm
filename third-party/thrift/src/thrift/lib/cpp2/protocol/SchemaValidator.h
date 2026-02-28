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

#pragma once

#include <thrift/lib/cpp2/dynamic/Path.h>
#include <thrift/lib/cpp2/protocol/detail/DynamicCursorSerializer.h>

#include <vector>

namespace apache::thrift {

/**
 * Result of schema validation for a serialized blob.
 *
 * Note: Validation only checks wire format compatibility, not semantic
 * identity. A result of Maybe or MaybeWithUnknownFields indicates the blob
 * can be deserialized according to the schema, but does not guarantee the
 * data was produced with that schema in mind. It is a core part of Thrift's
 * model that the type of a payload is known by both reader and writer, and as
 * such positively determining the type of a payload from the contents is not
 * possible.
 */
enum class SchemaValidationResult {
  // The serialized blob may have been produced by the schema
  Maybe,
  // The serialized blob may have been produced by the schema but contains
  // unknown fields (only applicable to structured types)
  MaybeWithUnknownFields,
  // Validation failed: type mismatch detected, blob not fully consumed,
  // or the blob could not be parsed according to the expected type
  No,
};

/**
 * Information about a field that exists in the blob but not in the schema.
 */
struct UnknownFieldInfo {
  // Path to the parent struct containing the unknown field
  dynamic::Path parentPath;
  // Field identifier (field ID or name if available)
  std::string fieldIdentifier;
};

/**
 * Information about a field where the wire type doesn't match the schema type.
 */
struct MismatchedFieldInfo {
  // Path to the mismatched field
  dynamic::Path path;
  // Human-readable description of the expected type
  std::string expectedType;
  // Human-readable description of the actual type
  std::string actualType;
};

/**
 * Detailed result of schema validation including paths to problematic fields.
 */
struct SchemaValidationResultWithPaths {
  SchemaValidationResult result;

  // Fields that exist in the blob but not in the schema
  std::vector<UnknownFieldInfo> unknownFields;

  // Fields where the wire type doesn't match the schema type
  std::vector<MismatchedFieldInfo> mismatchedFields;
};

/**
 * Validates that a serialized blob conforms to a TypeRef schema.
 *
 * This function validates any TypeRef type (structured, container, or
 * primitive) and ensures the entire blob is consumed during validation.
 *
 * @param serializedData The serialized blob to validate
 * @param typeRef The TypeRef schema to validate against
 * @return SchemaValidationResult indicating:
 *   - Maybe: The blob conforms to the schema and was fully consumed
 *   - MaybeWithUnknownFields: The blob conforms but contains unknown fields
 *   - No: Type mismatch, blob not fully consumed, or parse error
 */
template <typename ProtocolReader>
SchemaValidationResult validateBlob(
    const folly::IOBuf& serializedData, const type_system::TypeRef& typeRef);

/**
 * Validates that a serialized blob conforms to a TypeRef schema.
 * Returns detailed information about validation failures including paths.
 *
 * @param serializedData The serialized blob to validate
 * @param typeRef The TypeRef schema to validate against
 * @return SchemaValidationResultWithPaths containing:
 *   - result: The overall validation result
 *   - unknownFieldPaths: Paths to fields in the blob not found in the schema
 *   - mismatchedFieldPaths: Paths to fields with type mismatches
 */
template <typename ProtocolReader>
SchemaValidationResultWithPaths validateBlobWithPaths(
    const folly::IOBuf& serializedData, const type_system::TypeRef& typeRef);

namespace detail {

/**
 * Optional state object for tracking paths during validation.
 * When provided, the validation functions will record paths to unknown
 * and mismatched fields.
 */
struct ValidationState {
  dynamic::PathBuilder& pathBuilder;
  std::vector<UnknownFieldInfo>& unknownFields;
  std::vector<MismatchedFieldInfo>& mismatchedFields;

  ValidationState(
      dynamic::PathBuilder& pb,
      std::vector<UnknownFieldInfo>& unknown,
      std::vector<MismatchedFieldInfo>& mismatched)
      : pathBuilder(pb), unknownFields(unknown), mismatchedFields(mismatched) {}
};

// Forward declarations for internal functions
template <typename ProtocolReader>
SchemaValidationResult validateStructuredImpl(
    StructuredDynamicCursorReader<ProtocolReader, false>& reader,
    const type_system::TypeRef& typeRef,
    ValidationState* state = nullptr);

template <typename ProtocolReader>
SchemaValidationResult validateContainer(
    ContainerDynamicCursorReader<ProtocolReader, false>& reader,
    const type_system::TypeRef& containerTypeRef,
    ValidationState* state = nullptr);

/**
 * Helper to validate a primitive type and skip the value.
 */
template <typename TypeTag, typename Reader>
SchemaValidationResult validatePrimitive(
    Reader& reader,
    protocol::TType actualType,
    const type_system::TypeRef& expectedTypeRef,
    ValidationState* state) {
  try {
    ensureTypesCompatible<TypeTag>(actualType);
    reader.skip();
    return SchemaValidationResult::Maybe;
  } catch (const std::runtime_error&) {
    if (state) {
      state->mismatchedFields.push_back(
          MismatchedFieldInfo{
              state->pathBuilder.path(),
              dynamic::detail::typeDisplayName(expectedTypeRef),
              fmt::format("{}", actualType)});
    }
    return SchemaValidationResult::No;
  }
}

/**
 * Helper function to validate a single element against a TypeRef schema.
 * Works with both StructuredDynamicCursorReader and
 * ContainerDynamicCursorReader.
 */
template <typename Reader>
SchemaValidationResult validateElement(
    Reader& reader,
    const type_system::TypeRef& elementTypeRef,
    protocol::TType actualType,
    ValidationState* state = nullptr) {
  if (elementTypeRef.isStructured()) {
    if (actualType != protocol::TType::T_STRUCT) {
      if (state) {
        state->mismatchedFields.push_back(
            MismatchedFieldInfo{
                state->pathBuilder.path(),
                dynamic::detail::typeDisplayName(elementTypeRef),
                fmt::format("{}", actualType)});
      }
      return SchemaValidationResult::No;
    }
    auto childReader = reader.beginReadStructured();
    auto result = validateStructuredImpl(childReader, elementTypeRef, state);
    reader.endRead(std::move(childReader));
    return result;
  } else if (
      elementTypeRef.isList() || elementTypeRef.isSet() ||
      elementTypeRef.isMap()) {
    auto childReader = reader.beginReadContainer();
    auto result = validateContainer(childReader, elementTypeRef, state);
    reader.endRead(std::move(childReader));
    return result;
  } else if (elementTypeRef.isAny()) {
    if (actualType != protocol::TType::T_STRUCT) {
      if (state) {
        state->mismatchedFields.push_back(
            MismatchedFieldInfo{
                state->pathBuilder.path(),
                dynamic::detail::typeDisplayName(elementTypeRef),
                fmt::format("{}", actualType)});
      }
      return SchemaValidationResult::No;
    }
    // In the future we could recursively validate the contents of the Any
    reader.skip();
    return SchemaValidationResult::Maybe;
  } else if (elementTypeRef.isOpaqueAlias()) {
    // Validate against the underlying target type
    return validateElement(
        reader, elementTypeRef.asOpaqueAlias().targetType(), actualType, state);
  } else if (elementTypeRef.isEnum()) {
    return validatePrimitive<type::i32_t>(
        reader, actualType, elementTypeRef, state);
  } else if (elementTypeRef.isBool()) {
    return validatePrimitive<type::bool_t>(
        reader, actualType, elementTypeRef, state);
  } else if (elementTypeRef.isByte()) {
    return validatePrimitive<type::byte_t>(
        reader, actualType, elementTypeRef, state);
  } else if (elementTypeRef.isI16()) {
    return validatePrimitive<type::i16_t>(
        reader, actualType, elementTypeRef, state);
  } else if (elementTypeRef.isI32()) {
    return validatePrimitive<type::i32_t>(
        reader, actualType, elementTypeRef, state);
  } else if (elementTypeRef.isI64()) {
    return validatePrimitive<type::i64_t>(
        reader, actualType, elementTypeRef, state);
  } else if (elementTypeRef.isFloat()) {
    return validatePrimitive<type::float_t>(
        reader, actualType, elementTypeRef, state);
  } else if (elementTypeRef.isDouble()) {
    return validatePrimitive<type::double_t>(
        reader, actualType, elementTypeRef, state);
  } else if (elementTypeRef.isString()) {
    return validatePrimitive<type::string_t>(
        reader, actualType, elementTypeRef, state);
  } else if (elementTypeRef.isBinary()) {
    return validatePrimitive<type::binary_t>(
        reader, actualType, elementTypeRef, state);
  } else {
    folly::throw_exception<std::runtime_error>(
        "Unsupported TypeRef kind in schema validation");
  }
}

/**
 * Helper function to validate a container type against a TypeRef schema.
 */
template <typename ProtocolReader>
SchemaValidationResult validateContainer(
    ContainerDynamicCursorReader<ProtocolReader, false>& reader,
    const type_system::TypeRef& containerTypeRef,
    ValidationState* state) {
  bool hasUnknownFields = false;
  std::size_t index = 0;

  // Helper to validate an element with optional path tracking
  auto validateWithPath = [&](auto&& enterPath) -> SchemaValidationResult {
    auto elementTypeRef = reader.nextTypeRef();
    DCHECK(elementTypeRef);
    protocol::TType actualType = reader.nextTType();

    if (state) {
      auto guard = enterPath();
      return validateElement(reader, *elementTypeRef, actualType, state);
    }
    return validateElement(reader, *elementTypeRef, actualType, state);
  };

  while (reader.remaining() > 0) {
    SchemaValidationResult result;

    if (state) {
      if (containerTypeRef.isList()) {
        result = validateWithPath(
            [&] { return state->pathBuilder.enterListElement(index); });
      } else if (containerTypeRef.isSet()) {
        result = validateWithPath(
            [&] { return state->pathBuilder.enterSetElement(index); });
      } else if (containerTypeRef.isMap()) {
        // For maps, alternate between key and value
        if (index % 2 == 0) {
          result = validateWithPath(
              [&] { return state->pathBuilder.enterMapKey(index / 2); });
        } else {
          result = validateWithPath(
              [&] { return state->pathBuilder.enterMapValue(index / 2); });
        }
      } else {
        // Unknown container type - just validate without path
        auto elementTypeRef = reader.nextTypeRef();
        DCHECK(elementTypeRef);
        protocol::TType actualType = reader.nextTType();
        result = validateElement(reader, *elementTypeRef, actualType, state);
      }
    } else {
      auto elementTypeRef = reader.nextTypeRef();
      DCHECK(elementTypeRef);
      protocol::TType actualType = reader.nextTType();
      result = validateElement(reader, *elementTypeRef, actualType, state);
    }

    if (result == SchemaValidationResult::No) {
      return SchemaValidationResult::No;
    }
    if (result == SchemaValidationResult::MaybeWithUnknownFields) {
      hasUnknownFields = true;
    }
    ++index;
  }

  return hasUnknownFields ? SchemaValidationResult::MaybeWithUnknownFields
                          : SchemaValidationResult::Maybe;
}

/**
 * Internal implementation for validating structured types.
 */
template <typename ProtocolReader>
SchemaValidationResult validateStructuredImpl(
    StructuredDynamicCursorReader<ProtocolReader, false>& reader,
    const type_system::TypeRef& typeRef,
    ValidationState* state) {
  if (!typeRef.isStructured()) {
    if (state) {
      state->mismatchedFields.push_back(
          MismatchedFieldInfo{
              state->pathBuilder.path(),
              dynamic::detail::typeDisplayName(typeRef),
              "T_STRUCT"});
    }
    return SchemaValidationResult::No;
  }

  const auto& structuredNode = typeRef.asStructured();
  bool hasUnknownFields = false;

  while (reader.fieldType() != protocol::TType::T_STOP) {
    int16_t fieldId = reader.fieldId();
    protocol::TType fieldTType = reader.fieldType();

    // Check if field exists in schema
    if (fieldId == 0 || !structuredNode.hasField(type::FieldId{fieldId})) {
      // Unknown field detected
      if (state) {
        state->unknownFields.push_back(
            UnknownFieldInfo{
                state->pathBuilder.path(), fmt::format("{}", fieldId)});
      }
      hasUnknownFields = true;
      reader.skip();
      continue;
    }

    // Get expected field type from schema
    auto fieldTypeRef = reader.fieldTypeRef();
    DCHECK(fieldTypeRef);

    SchemaValidationResult result;
    if (state) {
      // Get field name for path
      auto handle = structuredNode.fieldHandleFor(type::FieldId{fieldId});
      const auto& fieldDef = structuredNode.at(handle);
      auto guard = state->pathBuilder.enterField(fieldDef.identity().name());
      result = validateElement(reader, *fieldTypeRef, fieldTType, state);
    } else {
      result = validateElement(reader, *fieldTypeRef, fieldTType, state);
    }

    if (result == SchemaValidationResult::No) {
      return SchemaValidationResult::No;
    }
    if (result == SchemaValidationResult::MaybeWithUnknownFields) {
      hasUnknownFields = true;
    }
  }

  return hasUnknownFields ? SchemaValidationResult::MaybeWithUnknownFields
                          : SchemaValidationResult::Maybe;
}

} // namespace detail

namespace detail {

/**
 * Maps a TypeRef to its expected TType.
 */
inline protocol::TType getExpectedTType(const type_system::TypeRef& typeRef) {
  if (typeRef.isStructured() || typeRef.isAny()) {
    return protocol::TType::T_STRUCT;
  } else if (typeRef.isList()) {
    return protocol::TType::T_LIST;
  } else if (typeRef.isSet()) {
    return protocol::TType::T_SET;
  } else if (typeRef.isMap()) {
    return protocol::TType::T_MAP;
  } else if (typeRef.isBool()) {
    return protocol::TType::T_BOOL;
  } else if (typeRef.isByte()) {
    return protocol::TType::T_BYTE;
  } else if (typeRef.isI16()) {
    return protocol::TType::T_I16;
  } else if (typeRef.isI32() || typeRef.isEnum()) {
    return protocol::TType::T_I32;
  } else if (typeRef.isI64()) {
    return protocol::TType::T_I64;
  } else if (typeRef.isFloat()) {
    return protocol::TType::T_FLOAT;
  } else if (typeRef.isDouble()) {
    return protocol::TType::T_DOUBLE;
  } else if (typeRef.isString() || typeRef.isBinary()) {
    return protocol::TType::T_STRING;
  } else {
    throw std::runtime_error("Unsupported TypeRef kind in schema validation");
  }
}

/**
 * Internal implementation shared by validateBlob and validateBlobWithPaths.
 */
template <typename ProtocolReader>
SchemaValidationResult validateBlobImpl(
    const folly::IOBuf& serializedData,
    const type_system::TypeRef& typeRef,
    ValidationState* state) {
  ProtocolReader reader;
  reader.setInput(&serializedData);

  if (typeRef.isOpaqueAlias()) {
    return validateBlobImpl<ProtocolReader>(
        serializedData, typeRef.asOpaqueAlias().targetType(), state);
  }

  protocol::TType expectedTType = getExpectedTType(typeRef);

  // For structured types, perform deep validation
  if (typeRef.isStructured()) {
    using ProtocolWriter = std::conditional_t<
        std::is_same_v<ProtocolReader, CompactProtocolReader>,
        CompactProtocolWriter,
        BinaryProtocolWriter>;

    DynamicCursorSerializationWrapper<ProtocolReader, ProtocolWriter> wrapper(
        serializedData.clone(), typeRef);
    auto structReader = wrapper.beginRead();
    auto result = validateStructuredImpl(structReader, typeRef, state);
    wrapper.endRead(std::move(structReader));

    if (result == SchemaValidationResult::No) {
      return SchemaValidationResult::No;
    }

    // Check that entire blob was consumed
    reader.skip(expectedTType);
    if (reader.getCursor().totalLength() != 0) {
      return SchemaValidationResult::No;
    }

    return result;
  }

  // For non-structured types, skip and check consumption
  try {
    reader.skip(expectedTType);
  } catch (...) {
    return SchemaValidationResult::No;
  }

  if (reader.getCursor().totalLength() != 0) {
    return SchemaValidationResult::No;
  }

  return SchemaValidationResult::Maybe;
}

} // namespace detail

template <typename ProtocolReader>
SchemaValidationResult validateBlob(
    const folly::IOBuf& serializedData, const type_system::TypeRef& typeRef) {
  return detail::validateBlobImpl<ProtocolReader>(
      serializedData, typeRef, nullptr);
}

template <typename ProtocolReader>
SchemaValidationResultWithPaths validateBlobWithPaths(
    const folly::IOBuf& serializedData, const type_system::TypeRef& typeRef) {
  SchemaValidationResultWithPaths resultWithPaths;
  resultWithPaths.result = SchemaValidationResult::Maybe;

  if (typeRef.isOpaqueAlias()) {
    return validateBlobWithPaths<ProtocolReader>(
        serializedData, typeRef.asOpaqueAlias().targetType());
  }

  if (typeRef.isStructured()) {
    // Create path builder with the root type
    dynamic::PathBuilder pathBuilder(typeRef);
    detail::ValidationState state(
        pathBuilder,
        resultWithPaths.unknownFields,
        resultWithPaths.mismatchedFields);

    resultWithPaths.result = detail::validateBlobImpl<ProtocolReader>(
        serializedData, typeRef, &state);
  } else {
    resultWithPaths.result = detail::validateBlobImpl<ProtocolReader>(
        serializedData, typeRef, nullptr);
  }

  return resultWithPaths;
}

} // namespace apache::thrift
