# Terse Write

## Overview

A [terse](../idl/field-qualifiers#fields-annotated-with-thrifttersewrite) field is a field annotated with `@thrift.TerseWrite`. The annotation has similar semantics to a field qualifier where it changes the field semantic to be terse. It is designed to achieve the followings:

* save network bandwidth
* reduce memory usage by eliminating needs of isset bit
* reduce the need for `optional` keyword that does not need to distinguish unset and [intrinsic default value](../idl/#intrinsic-default-values)
* replace nuanced backfill semantic from the custom default of `unqualified` fields
* replace ill-formed deprecated `terse_write` compiler option version

If a terse field equals to the [intrinsic default value](../idl/#default-values), it will be skipped during serialization. This differs from `optional` field that is serialized if it is explicitly set or `unqualified` field that is always serialized. A terse field will be cleared to the intrinsic default value, ignoring the [custom default value](../idl/#default-values) if exist. Please refer to the rest of the guide for more detail.

:::note
Setting a terse field to the intrinsic default value is effectively identical to clearing the field.
:::

## Usage Guide

:::note
Include `thrift/annotation/thrift.thrift` file before using `@thrift.TerseWrite` annotation.
:::

### Annotating `@thrift.TerseWrite` on Package

This will effectively promote all unqualified fields in the current package to be terse fields.

```
include "thrift/annotation/thrift.thrift"

@thrift.TerseWrite
package "facebook.com/thrift/test"

struct Foo {
  1: i32 field1; // promoted to terse
  2: optional i32 field2; // keep optional
  3: required i32 field3; // keep required
}
```

#### Annotating `@thrift.TerseWrite` on Struct or Exception

This will effectively promote all unqualified fields in the current struct to be terse fields.

```
include "thrift/annotation/thrift.thrift"

package "facebook.com/thrift/test"

@thrift.TerseWrite
struct Foo {
  1: i32 field1; // promoted to terse
  2: optional i32 field2; // keep optional
  3: required i32 field3; // keep required
}

@thrift.TerseWrite
struct FooException {
  1: i32 field1; // promoted to terse
  2: optional i32 field2; // keep optional
  3: required i32 field3; // keep required
}
```

### Annotating `@thrift.TerseWrite` on Field

This promotes an annotated unqualified field to terse field. Note, you will get a Thrift compiler error if you try to annotate `optional` or `required` fields.

```
include "thrift/annotation/thrift.thrift"

package "facebook.com/thrift/test"

struct Foo {
  1: i32 field1; // keep unqualified
  @thrift.TerseWrite
  2: optional i32 field2; // Thrift compiler error
  @thrift.TerseWrite
  3: required i32 field3; // Thrift compiler error
  @thrift.TerseWrite
  4: i32 field4; // promoted to terse
}
```

### Annotating `@thrift.TerseWrite` on Thrift Union

Union is a special structured type where a single field can be set at any time. Therefore, fields inside union can be treated as if they have optional qualifiers. Therefore, `@thrift.TerseWrite` cannot annotate a union or fields inside union, and it will produce a Thrift compiler error.

```
include "thrift/annotation/thrift.thrift"

package "facebook.com/thrift/test"

union Foo {
  @thrift.TerseWrite
  1: i32 field1; // Thrift compiler error
}
```

### Unqualified VS Terse

Use `unqualified` field if you need to distinguish whether a field was explicitly set or not. `unqualified` field is always serialized regardless whether it was explicitly set or unset. This may be desirable to verify whether the client’s schema included the field.

Use `unqualified` field if a field has custom default, which implies that a field will not likely have the intrinsic default value unless if it was intentionally cleared. This also applies if you are certain that a field will not likely have the intrinsic default value, avoid using a terse field.

Otherwise, use `terse` field as it will reduce the network bandwidth and reduce memory usage.

### Optional VS Terse

Both `optional` and terse field can be used to save the network bandwidth, as both of them skip serialization for certain condition. Use `optional` field if you need to distinguish whether a field was explicitly set or not. For example, for an `optional` bool field can exist in three states `unset`, `false`, or `true`. However, a terse bool field can exist in two states `false` or `true`. Therefore, `optional` qualifier must be used if you want to represent `unset`.

### Custom Default

Custom defaults are generally discouraged for terse fields, since the usage of custom default indicates that the field is less likely to be empty.

```
include "thrift/annotation/thrift.thrift"

package "facebook.com/thrift/test"

struct Foo {
  @thrift.TerseWrite
  1: i32 terse_field = 42;
  2: i32 unqualified_field = 43;
}
```

When struct `Foo` is constructed, the field `field1` will have the custom default value. Unless the field is explicitly set to the intrinsic default (i.e. 0), it will always be serialized.

Another thing to note is that custom defaults are also ignored during deserialization, since the deserializer cannot distinguish if the field is missing from the schema version mismatch or if the field was skipped during the serialization. Therefore, the deserializer will always set the field to the intrinsic default value when the field does not exist in the serialized binary. This differ from the unqualified field where custom default values can be use to backfill the missing field with some custom values.

```
// Example in C++
// default constructed
Foo foo1;
EXPECT_EQ(foo1.terse_field(), 42);
EXPECT_EQ(foo1.unqualified_field(), 43);

// constructed from deserialization
// imagine the client did not have both terse_field and unqualified_field
// in the schema and sent empty struct over the wire.
Foo foo2 = apache::thrift::CompactSerializer::deserialize(serialized_binary);
EXPECT_EQ(foo2.terse_field(), 0);
EXPECT_EQ(foo2.unqualified_field(), 43);
```

### Terse Struct Fields

```
include "thrift/annotation/thrift.thrift"

package "facebook.com/thrift/test"

struct Foo {
  1: i32 field1;
  2: optional i32 field2;
  3: required i32 field3;
  @thrift.TerseWrite
  4: i32 field4;
}

struct Bar {
  @thrift.TerseWrite
  1: Foo field1; // This terse struct field can never be empty.
}
```

A terse struct is empty iff all nested fields are empty. Nested optional fields should be unset and terse fields should be empty. Note that if a terse struct has unqualified or required fields, it can never be empty.

:::note

A `Bar.field1` is a terse struct field, and the struct `Foo` is not emptiable and can never be skipped for serialization due to `unqualified` `Foo.field1` and `required` `Foo.field3`.

:::

### Difference with compiler option version

:::note

The compiler option version `deprecated_terse_writes` is effectively deprecated.

:::

Thrift C++ had a compiler flag [deprecated_terse_writes](../idl/field-qualifiers#terse-writes-compiler-option) that had a similar semantic to a terse field. However, the difference is that the compiler option version only support primitive and container types, and it skips serialization when a field is equal to the [custom default value](../idl/#default-values) instead of intrinsic default value. This makes it inconsistent and bug-prone, as the custom default can be changed and differ from binary to binary.
