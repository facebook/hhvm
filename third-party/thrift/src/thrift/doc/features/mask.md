---
state: beta
description: Mask fields in a thrift struct
---

# Thrift Mask

## Overview

Thrift Mask represents a data structure for managing subsets of fields, map elements (with integer and string keys), and types in [Thrift Any](./any). It provides operations, such as clearing specific fields from a Thrift object. While the data structure is available across all supported languages, the utility functions are currently implemented only in C++.

:::note
Thrift Mask only supports integer or string keys for map elements. For all other map key types, Thrift Mask can not represent subset of map elements and only supports granularity of map itself as a field.
:::

## Motivation
When a client doesn't need all fields, the server can only return selected fields to the client to reduce network bandwidth.

## Data Structure

Thrift Mask can be either an inclusive mask or an exclusive mask — fields or nested field masks that are included or excluded. The nested masks are used for masking the nested fields. The definition can be found in [`thrift/lib/thrift/field_mask.thrift`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/field_mask.thrift).
```thrift
union Mask {
  1: map<i16, Mask> excludes; // Fields that will be excluded
  2: map<i16, Mask> includes; // Fields that will be included
}

const Mask allMask = {"excludes": {}}; // Masks all fields, map elements, types in Thrift Any.
const Mask noneMask = {"includes": {}}; // Masks no fields, map elements, types in Thrift Any.

```
[Debug protocol](/features/serialization/protocols.md#debug-protocol) can be used to convert Mask to a human readable string.

## Operations

Operations are defined in [`thrift/lib/cpp2/protocol/FieldMask.h`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp2/protocol/FieldMask.h) and support both schema-full (generated Thrift Struct) and schema-less (protocol::Object) Thrift structs:

```cpp
// Constructs a new field mask that is reverse of the given mask.
Mask reverseMask(Mask);

// Whether a field mask is compatible with Thrift struct (the masked fields exist
// in ThriftStruct)
bool is_compatible_with<ThriftStruct>(Mask);

// Ensure all masked fields are set in Thrift struct.
void ensure(Mask, ThriftStruct);

// Remove all masked fields in Thrift struct.
void clear(Mask, ThriftStruct);

// Returns a new object that contains only the masked fields.
ThriftStruct filter(Mask, ThriftStruct src);

// It also provides support for protocol::Object
void clear(Mask, protocol::Object);
protocol::Object filter(Mask, protocol::Object src);
```

## MaskBuilder

`MaskBuilder` serves as a wrapper for Thrift Mask, providing type validation and convenient methods for mask population. It is bound to a specific Thrift struct type and offers APIs for including and excluding fields, map elements, and types with field names and field identifiers like idents.

```cpp
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
    MaskBuilder& includes_map_element<Id...>(int64_t key, const Mask& mask = allMask());
    MaskBuilder& includes_map_element<Id...>(std::string key, const Mask& mask = allMask());
    MaskBuilder& includes_type<Id...>(type::Type type, const Mask& mask = allMask());

    MaskBuilder& excludes<Id...>(const Mask& mask = allMask());
    MaskBuilder& excludes_map_element<Id...>(int64_t key, const Mask& mask = allMask());
    MaskBuilder& excludes_map_element<Id...>(std::string key, const Mask& mask = allMask());
    MaskBuilder& excludes_type<Id...>(type::Type type, const Mask& mask = allMask());

    // Mask APIs
    void ensure(Struct& obj) const;
    void clear(Struct& obj) const;
    Struct filter(const Struct& src) const;
}
```

For a given Thrift struct,

```thrift
struct MyStruct {
  1: i32 int_field;
  2: map<i32, string> i32_to_string_map;
  3: map<string, string> string_to_string_map;
  4: Any any_field;
}
```
`MaskBuilder` can be used as the following.

```cpp
// For inclusive mask, start with noneMask which masks no fields.
// If started with allMask, all includes API is no-op since allMask already includes them.
MaskBuilder<MyStruct> builder(noneMask());
builder.includes<ident::int_field>();
builder.includes_map_element<ident::i32_to_string_map>(42);
builder.includes_map_element<ident::string_to_string_map>("myKey");
builder.includes_type<ident::any_field>(type::Type::get<type::struct_t<MyOtherStruct>>());
Mask& mask = builder.toThrift();
```

It also provides `MaskAdapter`, which adapts `Mask` to be `MaskBuilder` in a thrift struct.

## Logical operators

The following logical operators are available for mask construction:

```cpp
Mask operator&(const Mask&, const Mask&); // intersect
Mask operator|(const Mask&, const Mask&); // union
Mask operator-(const Mask&, const Mask&); // subtract
```

## Compare

Constructs a Thrift Mask object that includes the fields that are different in the given two Thrift structs.

```
Mask compare(const Struct& original, const Struct& modified);
```

## Serialization with Mask

:::note
Partial serialization and deserialization features are limited to schema-less Protocol Objects with fields and map elements.
:::

### Overview
Thrift Mask enables partial serialization and deserialization of objects. Using `parseObject`, you can construct a Protocol Object containing only the masked fields and map elements. The remaining unmasked fields are stored in `MaskedProtocolData`. Later, you can use `serializeObject` with the `MaskedProtocolData` to reconstruct the complete object with all fields included.

```cpp
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

This is useful when updating a small part of the object from serialized data, as it doesn’t need to deserialize the entire object. The function `applyPatchToSerializedData` performs this by extracting a mask from Thrift Patch with only the updated fields.

```cpp
std::unique_ptr<folly::IOBuf> applyPatchToSerializedData<Protocol>(
    const protocol::Object& patch, const folly::IOBuf& buf);
```

If the excluded fields and map elements are not needed, `parseObjectWithoutExcludedData` can be used to only parse fields and map elements that are masked.

```cpp
template <typename Protocol>
Object parseObjectWithoutExcludedData(const folly::IOBuf& buf, const Mask& mask);
```
