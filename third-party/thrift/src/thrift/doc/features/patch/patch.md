---
state: experimental
---

# Patch

:::note

Thrift Patch is an experimental feature that is only implemented in C++.

:::

## Overview

How mutations for Thrift values are represented, manipulated, and applied. It can be considered a [diff](https://en.wikipedia.org/wiki/Diff) between two Thrift values.

## Motivation

1. Reduce network bandwidth.
    1. Considering that we have a service which stores thrift objects in database. When a client wants to modify the object, they can just send the Patch instead of the whole object to reduce network bandwidth.
    2. This is how Patch is used in Rodos and CPEntity.
2. Reduce storage overhead.
    1. When storing multiple similar thrift objects in database, we can store one root object, and patches to generate other objects from the root object.
    2. In addition, you might want to know how stored thrift objects are changed over time. You can store the changesets as patches.
    3. This is how Patch is used in Control Plane.

## Enable Patch

To enable Patch, `@patch.GeneratePatch` annotation **must** be used recursively on package or struct level.

There are 2 representations of a Patch — Static Patch and Dynamic Patch. Both have identical wire format, though they have different requirements and provide different APIs. In general, Static Patch is preferred unless you can not include the generated thrift header.

* Static Patch, or Schema-full Patch, is generated from Thrift codegen and has 1:1 mapping to Thrift type.
    * Pros: More user-friendly APIs (e.g., be able to modify patch based on field name, user can’t generate invalid patch).
    * Cons: Requires thrift file to be available (e.g., `<thrift_file>_types.h` must be included in C++).
* Dynamic Patch, or Schema-less Patch, is a schema-less representation of static patch that is consumed with dynamic type `protocol::Object`.
    * Pros: Can be used without thrift file.
    * Cons: Less user-friendly APIs.

## C++ Static Patch

### Struct Patch

Considering the following thrift struct.

```
// In thrift file
struct MyStruct {
  1: optional string field;
};
```

Then you can use `MyStructPatch` to access the underlying [StructPatch](../../../ref/cpp/class/apache/thrift/op/detail/StructPatch). e.g.,

```
// In C++ file
namespace op = apache::thrift::op;
namespace ident = apache::thrift::ident;

MyStruct s;
s.field() = "hi";

MyStructPatch patch;
op::StringPatch& stringPatch = patch.patch<ident::field>();
stringPatch.prepend("(");
stringPatch.append(")");
patch.apply(s);
EXPECT_EQ(s.field(), "(hi)");
```

Here `patch.patch<ident::field>()` is a [StringPatch](../../../ref/cpp/class/apache/thrift/op/detail/StringPatch). It’s worth noting that `patch` method will ensure the existence of the field. If field doesn’t exist, it will be set to intrinsic default first, e.g.,

```
s.clear();
patch.apply(s);
EXPECT_TRUE(s.field().has_value());
EXPECT_EQ(s.field(), "()");
```

If you don’t want to patch when the field doesn’t exist, you **should** use `patchIfSet` method instead.
Besides patching fields, you can also use `patch` to modify the whole struct. e.g.,

```
patch.clear();
patch.apply(s);
EXPECT_FALSE(s.field().has_value());
```

### Merge

All Patches have `merge` method, it’s guaranteed that the following code.

```
patch1.apply(v);
patch2.apply(v);
```

is equivalent to

```
patch1.merge(patch2);
patch1.apply(v);
```

### Patch as Thrift Struct

The Patch itself is a thrift struct that can be included in another thrift struct, e.g.

```
// In Foo.thrift
@patch.GeneratePatch
struct MyStruct {
  1: optional string field;
};

// In Patches.thrift
include "Foo.thrift"
struct Patches {
  1: Foo.MyStructPatch patch;
}
```

`Patches` works as a normal thrift structure which you can serialize/deserialize.

NOTE: Due to implementation limitation, you cannot include MyStructPatch in the same file — you have to include it in another file. In addition, you cannot use `@GeneratePatch` for struct that contains another Patch.

### Value Patches

Besides StringPatch, there are [other types of Patch](../../../cpp_api_toc). One example would be [MapPatch](../../../ref/cpp/class/apache/thrift/op/detail/MapPatch). Consider the following thrift struct.


```
// In thrift file
struct MyMapStruct {
  2: map<i64, string> nested;
};
```

You can patch elements by key, e.g.,

```
MyMapStructPatch patch;
auto &stringPatch = patch.patch<ident::nested>().patchByKey(42);
stringPatch.prepend("(");
stringPatch.append(")");

MyMapStruct s;
s.nested[42] = "hi";
patch.apply(s);
EXPECT_EQ(s.nested[42], "(hi)");
```

### Read patch's content

`Patch::customVisitor(Visitor)` can be used to inspect Patch. This API uses the Visitor pattern to describe how Patch is applied. For each operation that will be performed by the patch, the corresponding method (that matches the write API) will be invoked.

For example, let's assume you have the following thrift struct:

    struct MyClass {
      1: string foo;
      2: bool bar;
    }

and then you created the following patch:

    MyClassPatch patch;
    patch.patch<ident::bar>().invert();
    patch.patch<ident::bar>().invert();
    patch.patch<ident::foo>().append("_");

`patch.customVisit(v)` will invoke the following methods

    v.ensure<ident::foo>();
    v.ensure<ident::bar>();
    v.patchIfSet<ident::foo>(StringPatch::createAppend("_"));
    v.patchIfSet<ident::bar>(BoolPatch{});  // no-op since inverted twice

### Type alias for Patch type

You can get Patch type from original type, vice versa. e.g.,

```
static_assert(std::is_same_v<apache::thrift::op::patch_type<MyStruct>, MyStructPatch>);
static_assert(std::is_same_v<MyStruct, MyStructPatch::value_type>);
```

## C++ Dynamic Patch

If you can not include the generated thrift header, you can still serialize any abritrary patch to `protocol::Object` and apply it. Here is an example how to apply the dynamic patch:

```
MyStruct s;
s.field() = "hi";

MyStructPatch patch;
op::StringPatch& stringPatch = patch.patch<ident::field>();
stringPatch.prepend("(");
stringPatch.append(")");

protocol::Object dynamicStruct = apache::thrift::protocol::toObject(s);
protocol::Object dynamicPatch = apache::thrift::protocol::toObject(patch);
protocol::applyPatch(dynamicPatch, dynamicStruct);

EXPECT_EQ(dynamicStruct[FieldId{1}].as_string(), "(hi)");

```

You can find all dynamic patch APIs [here](../../../ref/cpp/file/thrift/lib/cpp2/protocol/Patch.h).
