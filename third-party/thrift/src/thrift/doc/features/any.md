---
state: beta
description: How to store any value
---

import Tabs from '@theme/Tabs';
import TabItem from '@theme/TabItem';

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

### Registering types and protocols

<Tabs
  groupId="register"
  defaultValue="C++"
  values={[
    {label: 'C++', value: 'C++'},
    {label: 'Hack', value: 'Hack'},
    {label: 'Python', value: 'Python'},
    {label: 'Java', value: 'Java'}
  ]}>
  <TabItem value="C++">

In C++, all types are registered along with a protocol/serializer pair. All registered serializers must either be owned by the `TypeRegistry` or out live all `TypeRegistry` that they have been registered with.

```cpp
// Generated registry with the Thrift generated types (in your dependency tree)
// and primitive types already registered
auto registry = TypeRegistry::generated();

// Registering `FollyToStringSerializer` for type `double`
test::FollyToStringSerializer<double_t> serializer;
treg.registerSerializer(serializer, double_t{});

// Registering type `list<double>` with compact protocol
StdSerializer<list_t<double_t>, type::StandardProtocol::Compact> serializer;
treg.registerSerializer(serializer, list_t<double_t>{});

```
  </TabItem>
  <TabItem value="Hack">

TypeRegistry in www uses `ThriftTypeInfo` class attribute. All classes with this attribute are registered automatically. Thrift generated classes/enums have this attribute enabled if a universal name is available for the corresponding Thrift type.

Custom protocol to serializer mapping is only needed when (de)serializing using `ThriftLazyAny` helpers. Use `ThriftLazyAny::setIdToSerializer` or `ThriftLazyAny::setStringToSerializer` mapping.

```php

// registers FooStruct in TypeRegistry
<<\ThriftTypeInfo(shape('uri' => 'facebook.com/my_unique_package/FooStruct'))>>
class FooStruct implements \IThriftSyncStruct, \IThriftStructMetadata{ ...
}

public function customProtocolRegistry(string $uri) [write_props]: classname<TProtocolWritePropsSerializer> {
...
}

$any_obj = ThriftLazyAny::fromObject<FooStruct>($foo_obj);
// Registers custom protocol registry for current Object
$any_obj->setStringToSerializer(customProtocolRegistry);
```
  </TabItem>
  <TabItem value="Python">

All thrift-python modules (in your dependency tree) are pre-registered in `OmniAnyRegistry`. To register types on demand, use `AnyRegistry`.

Custom Protocols are not yet supported in Python TypeRegistry.

```python
from thrift.python.any.omni_registry import OmniAnyRegistry
from thrift.python.any.any_registry import AnyRegistry

# Register your module
from your.module.path import thrift_types
AnyRegistry().register_module(thrift_types)

# Alternatively you can register a single type
AnyRegistry().register_type(thrift_types.MyStruct)
```
  </TabItem>
  <TabItem value="Java">
In Java, all generated types (in your dependency tree) with universal name are registered.

```java
import com.facebook.thrift.any.Any

class ThriftTest {
private static ByteBuf mySerializer(Object o) {...}

private static Object myDeserializer(TypeStruct typeStruct, ByteBuf data) {...}

// Register custom serializer and deserializer
Any.registerSerializer("my-serializer", ThriftTest::mySerializer);
Any.registerDeserializer("my-serializer", ThriftTest::myDeserializer);
}
```
Check [TypeRegistry.java](https://github.com/facebook/fbthrift/blob/main/thrift/lib/java/common/src/main/java/com/facebook/thrift/type/TypeRegistry.java) and [AbstractAny.java](https://github.com/facebook/fbthrift/blob/main/thrift/lib/java/runtime/src/main/java/com/facebook/thrift/any/AbstractAny.java) for details
  </TabItem>
</Tabs>


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

## Usage Guide

### Add Thrift Any in IDL
```
include "thrift/lib/thrift/any.thrift"

struct Foo {
1: any.Any any_field;
}

service ThriftStore {
   void put(string id, any.Any value);
   any.Any get(string id);
}

```

### Create an AnyStruct Object

<Tabs
  groupId="creation"
  defaultValue="C++"
  values={[
    {label: 'C++', value: 'C++'},
    {label: 'Hack', value: 'Hack'},
    {label: 'Python', value: 'Python'},
    {label: 'Java', value: 'Java'},
  ]}>
  <TabItem value="C++">

- **Using Type Registry, [recommended way for Thrift generated and primitive types] **
  - Ensure that type and protocol are registered.
```cpp
#include <thrift/lib/cpp2/type/TypeRegistry.h>

auto str = "MY STRING";
auto registry = apache::thrift::type::TypeRegistry::generated();
AnyData any_data = registry.store<apache::thrift::type::StandardProtocol::Compact>(str);

// get AnyStruct from AnyData
AnyStruct any_obj = any_data.toThrift();
```
- **Using Thrift provided Helpers**
  - Only supports Binary and Compact protocols

```cpp
#include <thrift/lib/cpp2/type/Any.h>

auto str = "MY STRING";

// AnyData is a wrapper around AnyStruct.
AnyData any_data = AnyData::toAny<StandardProtocol::Compact>(str);

// get AnyStruct from AnyData
AnyStruct any_obj = any_data.toThrift();
```

- Using raw structs (not recommended)
  - User is responsible to ensure that `AnyStruct` created using raw struct is valid and has correct type/protocol information stored in it.

```cpp
#include <thrift/lib/thrift/gen-cpp2/any_types.h>

auto str = "MY STRING";
CompactProtocolWriter writer;
folly::IOBufQueue queue(folly::IOBufQueue::cacheChainLength());
writer.setOutput(&queue);
::apache::thrift::op::encode<Tag>(writer, str);

// `Any` is a type-alias for `AnyStruct`
Any builder;
builder.data() = queue.moveAsValue();
builder.protocol() = StandardProtocol::Compact;
builder.type() = Type::get<string_t>();
```

  </TabItem>
  <TabItem value="Hack">

- **Using Thrift provided Helpers**
  - Only supports Binary, Compact and, SimpleJson protocols

```php
$str = "My String";
$lazy_any = ThriftLazyAny::fromObject<string>($str);
// For binary use `toAnyUsingBinarySerializer`, for compact/json use `toAny`
$any_obj = $lazy_any->toAny(apache_thrift_type_standard_StandardProtocol::SimpleJson);
```

- Using raw structs (not recommended)
  - User is responsible to ensure that `AnyStruct` created using raw struct is valid and has correct type/protocol information stored in it.

```php
$str = "My String";
$type  = ThriftTypeStructAdapter::fromHackType<string>();
$data = TBinarySerializer::serializeData($str, $type->toTypeSpec());
$protocol = apache_thrift_type_rep_ProtocolUnion::fromShape(shape(
            "standard" => apache_thrift_type_standard_StandardProtocol::Binary,
          ));
$any_obj = apache_thrift_type_AnyStruct::fromShape(
            shape(
              'type' =>  $type,
              'protocol' => $protocol,
              'data' => $data,
            )
          );
```
  </TabItem>
  <TabItem value="Python">

- **Using Type Registry, [recommended way for Thrift generated and primitive types] **
  - Ensure that type and protocol are registered.

```python
from thrift.python.any.omni_registry import OmniAnyRegistry
from your.module.path.thrift_types import MyStruct

my_struct = MyStruct()
my_any = AnyRegistry().store(my_struct)
```

  </TabItem>
  <TabItem value="Java">

- **Using Thrift provided Helpers**

```java
import com.facebook.thrift.any.Any

// store `TestStruct` object within Any
TestStruct st = new TestStruct.Builder().setBoolField(true).setIntField(9).build();
Any<TestStruct> any =
    new Any.Builder<>(st).setProtocol(StandardProtocol.COMPACT).useUri().build();
AnyStruct any_obj = any.getAny();
```
  </TabItem>
</Tabs>
