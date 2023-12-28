---
state: beta
description: How to store any value
---
# Thrift Any

## Overview

### Thrift Any

Thrift Any is a data structure that represents a serialized dynamic type, and contains enough information to be deserialized by the receiver into a specific type. It encapsulates a Thrift value whose type is not known at compile time. Unlike normal Thrift values, which have their type defined in the IDL/AST (such as a `.thrift` file), an `any` object stores the type of data as part of its value.

### Thrift SemiAny

Thrift SemiAny is an alternative to Thrift Any which is useful in situations where the type may not be known or may change frequently. A `SemiAny` consists of a blob of content, optional `protocol` information, and optional `type` information. This allows for greater flexibility in handling different types of data, including arbitrary bytes that may not even be Thrift-encoded.



### Thrift Any vs Other Dynamic Types

| Dynamic Type | Description | Data Representation | Comparison with Thrift Any |
| :--: | :--- | :----- | :------------------------------------ |
| Protocol Object | Schemaless representation of Thrift types. Protocol Object focuses on parsing and serializing to and from a serialized thrift struct blob without schema.| A thrift union that contains unserialized schemaless types i.e. stores native values as union fields | Thrift Any is a serialized format that has a fixed type schema, whereas Protocol Object contains unserialized data that does not have any schema. Thrift Any has a lower overhead because it allows for the value to be serialized and stored in an `any` object without the need for additional deserialization or reserialization when sending it over the wire. This is particularly useful when the receiver of dynamic type simply wants to forward the serialized data, as it eliminates the need for additional processing steps.|
| `binary` fields| send serialized data over the wire. Thrift provides no additional methods to deserialize them. | Serialized blob | Using `binary` fields can lead to hardcoded assumptions or the need for custom structs that would resemble AnyStruct. Compared to using a binary field, Any is a standardized type which allows the intent to be clear, and avoids informal ways of determining the choice of protocol or which type is present. |


An API producing an `Any` is automatically better documented at baseline before the first docstring is produced.
