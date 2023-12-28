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


## Thrift Structs for Any and SemiAny

Both Thrift Any and SemiAny are represented using Thrift structs.
See Thrift file [`thrift/lib/thrift/any.thrift`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/any.thrift) for details.

Both the structs have three fields. A simplified version of these structs look something like this -
```
struct AnyStruct {
  1: TypeStruct type
  2: ProtocolUnion protocol
  3: binary data
}
```

### Type

Thrift Any can hold a variety of types, including primitive types, containers, and named types such as user-defined Thrift structs and enums.
The `type` field within the `AnyStruct` provides the `name` of the type and any additional context. For instance, when a user-defined type is serialized into an AnyStruct, its [Universal Name](universal-name.md) is stored in the `type` field. For container types, the `type` field also stores the type information for the parameters.

See `TypeStruct` defined in [`thrift/lib/thrift/type_rep.thrift`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/type_rep.thrift)) for details.

#### Type Registry

The Type Registry is a crucial component in the deserialization process of user-defined types stored in `AnyStruct`. It maintains a mapping from the universal name (stored in the `type` field) to its native generated type. This mapping allows the deserialization process to correctly interpret the data stored in AnyStruct and convert it back into its original user-defined type.

The Thrift generated structs, which have a universal name provided, are registered in the TypeRegistry as part of the initialization code.

:::note

Type Registry is available in C++, Hack, Python and Java. In C++, you need to add a compiler option `any` in order to generate intialization code to register Thrift types.
:::

### Protocol

Apart from the standard Thrift protocols, Thrift Any can also accommodate custom protocols for storing serialized data. These custom protocols can either be represented using a Universal Name or stored externally

See `ProtocolUnion` defined in [`thrift/lib/thrift/type_rep.thrift`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/thrift/type_rep.thrift)) for details.

When utilizing custom protocols, it is necessary to provide a mapping between the protocol name/id and the native serializer. This mapping is language-specific and depends on what API is used to interact with `AnyStruct`.
For eg: In C++, all custom protocols must be registered with the Type Registry when using it to load/store data within `AnyStruct`. However, when an `AnyStruct` is constructed/consumed directly, it is the user's responsibility to invoke the appropriate serializer based on the custom or standard protocol. This may involve writing a `switch` case to handle all the possible values in `ProtocolUnion`.

### Valid Any

An Any may be *empty*, in which case it contains **no value** (and has a type of **void**).

An Any which is *non-empty* -
* always knows which type it contains i.e. has complete valid type information
  * A complete `TypeStruct` contains non-empty `name` and correct number of params. Each param itself is a complete `TypeStruct`. For named types, it contains non-empty universal name of the user-defined struct.
* always contains sufficient information to deserialize the value, provided that the schema of the value’s type is known by the caller.


### SemiAny ↔ Any

SemiAny + Type Information = Any

* An Any can always be demoted to a corresponding SemiAny (SemiAny can represent a superset of what an Any can).
* A SemiAny can be promoted to an Any by providing a type or if the optional type information is present in the SemiAny.

### Helpers
Thrift Any and SemiAny types are universally available as they are no different than any other Thrift structs.

In addition, we also provide runtime helpers in certain languages as specified below.


| language | Helper |
| :--: | :--- |
| C++ | [Any.h](https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp2/type/Any.h)|
| Hack | [ThriftLazyAny](https://www.internalfb.com/code/www/flib/thrift/core/conformance/ThriftLazyAny.php?lines=140) |
| Python | [AnyRegistry](https://github.com/facebook/fbthrift/blob/main/thrift/lib/python/any/any_registry.py) |
| Java | [SerializedAny](https://github.com/facebook/fbthrift/blob/main/thrift/lib/java/runtime/src/main/java/com/facebook/thrift/any/SerializedAny.java) and [SerializedSemiAny](https://github.com/facebook/fbthrift/blob/main/thrift/lib/java/runtime/src/main/java/com/facebook/thrift/any/SerializedSemiAny.java)|
