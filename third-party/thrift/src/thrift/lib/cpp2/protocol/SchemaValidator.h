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

#include <thrift/lib/cpp2/protocol/detail/DynamicCursorSerializer.h>

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

namespace detail {

// Forward declarations for internal functions
template <typename ProtocolReader>
SchemaValidationResult validateStructuredImpl(
    StructuredDynamicCursorReader<ProtocolReader, false>& reader,
    const type_system::TypeRef& typeRef);

template <typename ProtocolReader>
SchemaValidationResult validateContainer(
    ContainerDynamicCursorReader<ProtocolReader, false>& reader,
    const type_system::TypeRef& containerTypeRef);

/**
 * Helper to validate a primitive type and skip the value.
 */
template <typename TypeTag, typename Reader>
SchemaValidationResult validatePrimitive(
    Reader& reader, protocol::TType actualType) {
  try {
    ensureTypesCompatible<TypeTag>(actualType);
    reader.skip();
    return SchemaValidationResult::Maybe;
  } catch (const std::runtime_error&) {
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
    protocol::TType actualType) {
  if (elementTypeRef.isStructured()) {
    if (actualType != protocol::TType::T_STRUCT) {
      return SchemaValidationResult::No;
    }
    auto childReader = reader.beginReadStructured();
    auto result = validateStructuredImpl(childReader, elementTypeRef);
    reader.endRead(std::move(childReader));
    return result;
  } else if (
      elementTypeRef.isList() || elementTypeRef.isSet() ||
      elementTypeRef.isMap()) {
    auto childReader = reader.beginReadContainer();
    auto result = validateContainer(childReader, elementTypeRef);
    reader.endRead(std::move(childReader));
    return result;
  } else if (elementTypeRef.isAny()) {
    if (actualType != protocol::TType::T_STRUCT) {
      return SchemaValidationResult::No;
    }
    // In the future we could recursively validate the contents of the Any
    reader.skip();
    return SchemaValidationResult::Maybe;
  } else if (elementTypeRef.isOpaqueAlias()) {
    // Validate against the underlying target type
    return validateElement(
        reader, elementTypeRef.asOpaqueAlias().targetType(), actualType);
  } else if (elementTypeRef.isEnum()) {
    return validatePrimitive<type::i32_t>(reader, actualType);
  } else if (elementTypeRef.isBool()) {
    return validatePrimitive<type::bool_t>(reader, actualType);
  } else if (elementTypeRef.isByte()) {
    return validatePrimitive<type::byte_t>(reader, actualType);
  } else if (elementTypeRef.isI16()) {
    return validatePrimitive<type::i16_t>(reader, actualType);
  } else if (elementTypeRef.isI32()) {
    return validatePrimitive<type::i32_t>(reader, actualType);
  } else if (elementTypeRef.isI64()) {
    return validatePrimitive<type::i64_t>(reader, actualType);
  } else if (elementTypeRef.isFloat()) {
    return validatePrimitive<type::float_t>(reader, actualType);
  } else if (elementTypeRef.isDouble()) {
    return validatePrimitive<type::double_t>(reader, actualType);
  } else if (elementTypeRef.isString()) {
    return validatePrimitive<type::string_t>(reader, actualType);
  } else if (elementTypeRef.isBinary()) {
    return validatePrimitive<type::binary_t>(reader, actualType);
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
    [[maybe_unused]] const type_system::TypeRef& containerTypeRef) {
  bool hasUnknownFields = false;

  while (reader.remaining() > 0) {
    auto elementTypeRef = reader.nextTypeRef();
    DCHECK(elementTypeRef);

    protocol::TType actualType = reader.nextTType();

    auto result = validateElement(reader, *elementTypeRef, actualType);
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

/**
 * Internal implementation for validating structured types.
 */
template <typename ProtocolReader>
SchemaValidationResult validateStructuredImpl(
    StructuredDynamicCursorReader<ProtocolReader, false>& reader,
    const type_system::TypeRef& typeRef) {
  if (!typeRef.isStructured()) {
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
      hasUnknownFields = true;
      reader.skip();
      continue;
    }

    // Get expected field type from schema
    auto fieldTypeRef = reader.fieldTypeRef();
    DCHECK(fieldTypeRef);

    auto result = validateElement(reader, *fieldTypeRef, fieldTType);
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

template <typename ProtocolReader>
SchemaValidationResult validateBlob(
    const folly::IOBuf& serializedData, const type_system::TypeRef& typeRef) {
  ProtocolReader reader;
  reader.setInput(&serializedData);

  if (typeRef.isOpaqueAlias()) {
    return validateBlob<ProtocolReader>(
        serializedData, typeRef.asOpaqueAlias().targetType());
  }

  protocol::TType expectedTType;
  if (typeRef.isStructured() || typeRef.isAny()) {
    expectedTType = protocol::TType::T_STRUCT;
  } else if (typeRef.isList()) {
    expectedTType = protocol::TType::T_LIST;
  } else if (typeRef.isSet()) {
    expectedTType = protocol::TType::T_SET;
  } else if (typeRef.isMap()) {
    expectedTType = protocol::TType::T_MAP;
  } else if (typeRef.isBool()) {
    expectedTType = protocol::TType::T_BOOL;
  } else if (typeRef.isByte()) {
    expectedTType = protocol::TType::T_BYTE;
  } else if (typeRef.isI16()) {
    expectedTType = protocol::TType::T_I16;
  } else if (typeRef.isI32() || typeRef.isEnum()) {
    expectedTType = protocol::TType::T_I32;
  } else if (typeRef.isI64()) {
    expectedTType = protocol::TType::T_I64;
  } else if (typeRef.isFloat()) {
    expectedTType = protocol::TType::T_FLOAT;
  } else if (typeRef.isDouble()) {
    expectedTType = protocol::TType::T_DOUBLE;
  } else if (typeRef.isString() || typeRef.isBinary()) {
    expectedTType = protocol::TType::T_STRING;
  } else {
    throw std::runtime_error("Unsupported TypeRef kind in schema validation");
  }

  // For structured types, perform deep validation
  if (typeRef.isStructured()) {
    using ProtocolWriter = std::conditional_t<
        std::is_same_v<ProtocolReader, CompactProtocolReader>,
        CompactProtocolWriter,
        BinaryProtocolWriter>;

    detail::DynamicCursorSerializationWrapper<ProtocolReader, ProtocolWriter>
        wrapper(serializedData.clone(), typeRef);
    auto structReader = wrapper.beginRead();
    auto result = detail::validateStructuredImpl(structReader, typeRef);
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

} // namespace apache::thrift
