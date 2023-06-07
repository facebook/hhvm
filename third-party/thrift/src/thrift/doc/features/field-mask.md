---
state: beta
description: Mask fields in a thrift struct
---

# FieldMask

## Overview

Field Mask is a data structure that represents a subset of fields and nested fields of a thrift struct, with utilities to manipulate these fields (e.g., copy certain fields from one thrift object to another).

## Motivation
When a client doesn't need all fields, the server can only return selected fields to the client to reduce network bandwidth.

## Data Structure

Field Mask can be either an inclusive mask or an exclusive mask — fields or nested field masks that are included or excluded. The nested masks are used for masking the nested fields. (Actual definition can be found here: [`thrift/lib/thrift/field_mask.thrift`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/field_mask.thrift).

```
union Mask {
  1: map<i16, Mask> excludes; // Fields that will be excluded
  2: map<i16, Mask> includes; // Fields that will be included
}

const Mask allMask = {"excludes": {}}; // Masks all fields/whole field.
const Mask noneMask = {"includes": {}}; // Masks no fields.`
```
[Debug protocol](/features/serialization/protocols.md#debug-protocol) can be used to convert Mask to a human readable string.

## APIs

Currently it provides the following APIs (APIs are in [`thrift/lib/cpp2/protocol/FieldMask.h`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp2/protocol/FieldMask.h)). These APIs are available for both schemaful (ThriftStruct) and schemaless (protocol::Object) Thrift structs.
[Package](../idl/#package-declaration) name must be defined in thrift file to use field mask APIs. It will give a compile-error without a package name.

```cpp
// Whether field mask is compatible with thrift struct (the masked fields exist
// in ThriftStruct)
bool is_compatible_with<ThriftStruct>(Mask);

// Call field_ref().ensure() for all masked fields and their precedents
void ensure(Mask, ThriftStruct);

// Remove all masked fields
void clear(Mask, ThriftStruct);

// Copy masked fields from src to dst. If some masked fields are not set in src,
// remove them in dst as well.
void copy(Mask, ThriftStruct src, ThriftStruct dst);

// It also provides support for protocol::Object
void clear(Mask, protocol::Object);
void copy(Mask, protocol::Object src, protocol::Object dst);
```

## MaskBuilder

MaskBuilder is a wrapper for Mask that works as a strongly typed mask. It is tied to a specific thrift struct type, and it provides APIs to add fields to the Mask with field names and field identifiers like idents.

```
struct MaskBuilder<Struct> {
    // All methods for constructing Mask return itself to chain
    MaskBuilder& reset_to_none();
    MaskBuilder& reset_to_all();
    MaskBuilder& invert();
    Mask& toThrift();

    // includes and excludes with field names and all ids
    MaskBuilder& includes(list<string> fieldNames, const Mask& mask = allMask());
    MaskBuilder& excludes(list<string> fieldNames, const Mask& mask = allMask());
    // Id can be Ordinal, FieldId, Ident, TypeTag, FieldTag
    MaskBuilder& includes<Id...>(const Mask& mask = allMask());
    MaskBuilder& excludes<Id...>(const Mask& mask = allMask());

    // reset_and_includes calls reset_to_none() and includes()
    // reset_and_excludes calls reset_to_all() and excludes()

    // Mask APIs
    void ensure(Struct& obj) const;
    void clear(Struct& obj) const;
    void copy(const Struct& src, Struct& dst) const;
}
```

It also provides `MaskAdapter`, which adapts `Mask` to be `MaskBuilder` in a thrift struct.

## Logical operators

These are logical operators to construct a new mask.

```
Mask operator&(const Mask&, const Mask&); // intersect
Mask operator|(const Mask&, const Mask&); // union
Mask operator-(const Mask&, const Mask&); // subtract
```

## Compare

Constructs a FieldMask object that includes the fields that are different in the given two Thrift structs.

```
Mask compare(const Struct& original, const Struct& modified);
```

## Serialization with Mask
Mask can be used with parseObject to construct a partial protocol::Object with only the masked fields. This also returns MaskedProtocolData, which stores the serialized data of the other fields, and serializeObject with MaskedProtocolData reserializes the object.
```
struct MaskedDecodeResult {
  Object included;
  MaskedProtocolData excluded;
};

// Only parses values that are masked. Unmasked fields are stored in MaskedProtocolData.
MaskedDecodeResult parseObject<Protocol>(const folly::IOBuf& buf, Mask mask);

// serialize the fields from the Object and MaskedProtocolData
std::unique_ptr<folly::IOBuf>
serializeObject<Protocol>(const protocol::Object& obj, MaskedProtocolData& protocolData);
```
This is useful when updating a small part of the object from serialized data, as it doesn’t need to deserialize the entire object. The function applyPatchToSerializedData performs this by extracting a mask from patch with only the updated fields.
```
std::unique_ptr<folly::IOBuf> applyPatchToSerializedData<Protocol>(
    const protocol::Object& patch, const folly::IOBuf& buf);
```
