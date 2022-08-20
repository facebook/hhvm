# Dynamic Values (Any)

## Any

How to store any value.

## Paths

How to access values from any Thrift value.

## FieldMask

### Overview

Field Mask is a data structure that represents a subset of fields and nested fields of a thrift struct, with utilities to manipulate these fields (e.g., copy certain fields from one thrift object to another).

### Motivation
When a client doesn't need all fields, the server can only return selected fields to the client to reduce network bandwidth.

### Data Structure

Field Mask can be either an inclusive mask or an exclusive mask — fields or nested field masks that are included or excluded. The nested masks are used for masking the nested fields. (Actual definition can be found here: [`thrift/lib/thrift/field_mask.thrift`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/field_mask.thrift).

```
union Mask {
  1: map<i16, Mask> excludes; // Fields that will be excluded
  2: map<i16, Mask> includes; // Fields that will be included
}

const Mask allMask = {"excludes": {}}; // Masks all fields/whole field.
const Mask noneMask = {"includes": {}}; // Masks no fields.`
```

## APIs

Currently it provides the following APIs (APIs are in [`thrift/lib/cpp2/FieldMask.h`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp2/FieldMask.h)). These APIs are available for both schemaful (ThriftStruct) and schemaless (protocol::Object) Thrift structs.
[Package](../definition/program/#packages) name must be defined in thrift file to use field mask APIs. It will give a compile-error without a package name.
```
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
Also, [debug protocol](../protocol/data/#debug-protocol) can be used to convert Mask to a human readable string.

## Protocol Object and Value

How to manipulate any value.
