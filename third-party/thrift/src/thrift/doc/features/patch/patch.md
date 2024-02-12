---
state: experimental
---

# Patch

:::note

Thrift Patch is an experimental feature that is under alpha testing. It's only implemented in C++. To use Patch, please reach out to the Thrift team with your use-case. You will also need to add your codebase to `PATCH_VISIBILITY` in `thrift/lib/thrift/TARGETS`.

:::

## Overview

How mutations for Thrift values are represented, manipulated, and applied. It can be considered a [diff](https://en.wikipedia.org/wiki/Diff) between two Thrift values.

## Motivation

1. Reduce network bandwidth.
    1. Assume that you have a service that stores Thrift objects in a database. When a client wants to modify an object, it can just send the Patch instead of the entire object to reduce network bandwidth.
    2. This is how Patch is used in Rodos and CPEntity.
2. Reduce storage overhead.
    1. When storing multiple similar Thrift objects in database, we can store one root object, and patches to generate other objects from the root object.
    2. In addition, you might want to know how stored Thrift objects are changed over time. You can store the changesets as patches.
    3. This is how Patch is used in Control Plane.

## Enable Patch

Use `thrift_patch_library` buck build rule to enable Patch. Internally, it will
create a `thrift_library` build rule that contains the patch structs in Thrift
file `[thrift_file_name]_patch.thrift`. It takes the following arguments

| Argument            | Description                                             | Example                            |
| ------------------- | ------------------------------------------------------- | ---------------------------------- |
| thrift_library_name | The name of thrift_library that contains the srcs.      | "my_rule"                          |
| thrift_library_srcs | The thrift source files that we want to generate Patch. | ["foo.thrift", "bar.thrift"]       |
| thrift_library_deps | The build rules that patch depends on.                  | ["//path/to/foo", "//path/to/bar"] |
| languages           | Same as the `languages` argument in thrift_library.     | ["cpp2", "python"]                 |

Any extra arguments will be forwarded to the internal Patch's thrift_library.

### Example

In TARGETS, for

```
thrift_library(
  name = "foo",
  languages = ['cpp2'],
  srcs = {"foo.thrift": []},
  deps = ["//path/to/bar"]
)
```

Add the following build rule to build patch target

```
thrift_patch_library(
  thrift_library_name = "foo",
  thrift_library_srcs = ["foo.thrift"],
  thrift_library_deps = ["//path/to/bar"],
  languages = ['cpp2'],
  thrift_cpp2_options = ['any'],
)
```

Internally, this will generate

```
thrift_library(
  name = "foo_patch",
  srcs = {"foo_patch.thrift": []},
  deps = [":foo", "//path/to/bar_patch"],
  languages = ['cpp2'],
  thrift_cpp2_options = ['any'],
)
```

So that this can be used later in C++

```cpp
// MyService.cpp
#include "path/to/gen-cpp2/foo_patch_types.h"

auto createPatch() {
  apache::thrift::op::patch_type<Foo> patch;
  patch.patch<ident::message>() += "suffix";
  return patch;
}
```

Dependencies can be included automatically via autodeps.

### Dynamic Patch

There are 2 representations of a Patch: Static Patch and Dynamic Patch. Both
have identical wire formats, though they have different requirements and provide
different APIs. In general, Static Patch is preferred unless you can not include
the generated Thrift header.

- Static Patch, or Schema-full Patch, is generated from Thrift codegen and has
  1:1 mapping to Thrift type.
  - Pros: More user-friendly APIs (e.g., ability to modify patch based on field
    name, user can’t generate invalid patch).
  - Cons: Requires Thrift file to be available (e.g.,
    `<thrift_file>_patch_types.h` must be included in C++).
    You need to recursively enable for all Thrift file dependencies.
- Dynamic Patch, or Schema-less Patch, is a schema-less representation of static
  patch that is consumed with dynamic type `protocol::Object`.
  - Pros: Can be used without Thrift file.
  - Cons: Less user-friendly APIs.

### Deprecated Workflow

There is also a deprecated workflow that can be used to enable Patch via the
`@patch.GeneratePatch` annotation.

When this annotation is used, the corresponding Patch struct will be generated directly in the original Thrift file.
This workflow is deprecated and it should not be used for new files.

## C++ Static Patch

### Struct Patch

Consider the following Thrift struct

```thrift
// In thrift file
struct MyStruct {
  1: optional string field;
};
```

Then you can use `MyStructPatch` to access the underlying [StructPatch](../../../ref/cpp/class/apache/thrift/op/detail/StructPatch). e.g.,

```cpp
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

Here `patch.patch<ident::field>()` is a [StringPatch](../../../ref/cpp/class/apache/thrift/op/detail/StringPatch). It’s worth noting that `patch` method will ensure the existence of the field. If field doesn’t exist, it will be set to the intrinsic default first, e.g.,

```cpp
s.clear();
patch.apply(s);
EXPECT_TRUE(s.field().has_value());
EXPECT_EQ(s.field(), "()");
```

If you don’t want to patch when the field doesn’t exist, you **should** use the `patchIfSet` method instead.
Besides patching fields you can also use `patch` to modify the whole struct. e.g.,

```cpp
patch.clear();
patch.apply(s);
EXPECT_FALSE(s.field().has_value());
```

### Merge

All Patches have `merge` method, it’s guaranteed that the following code

```cpp
patch1.apply(v);
patch2.apply(v);
```

is equivalent to

```cpp
patch1.merge(patch2);
patch1.apply(v);
```

### Value Patches

Besides StringPatch there are [other types of Patch](../../../cpp_api_toc). One example would be [MapPatch](../../../ref/cpp/class/apache/thrift/op/detail/MapPatch). Consider the following Thrift struct


```thrift
// In thrift file
struct MyMapStruct {
  2: map<i64, string> nested;
};
```

You can patch elements by key, e.g.,

```cpp
MyMapStructPatch patch;
auto &stringPatch = patch.patch<ident::nested>().patchByKey(42);
stringPatch.prepend("(");
stringPatch.append(")");

MyMapStruct s;
s.nested[42] = "hi";
patch.apply(s);
EXPECT_EQ(s.nested[42], "(hi)");
```

### Reading the Contents of a Patch

`Patch::customVisitor(Visitor)` can be used to inspect Patches. This API uses the Visitor pattern to describe how a Patch is applied. For each operation that will be performed by a patch, the corresponding method (that matches the write API) will be invoked.

For example, let's assume you have the following Thrift struct

```thrift
struct MyClass {
  1: string foo;
  2: bool bar;
}
```

and then you created the following patch

```cpp
MyClassPatch patch;
patch.patch<ident::bar>().invert();
patch.patch<ident::bar>().invert();
patch.patch<ident::foo>().append("_");
```

`patch.customVisit(v)` will invoke the following methods

```cpp
v.ensure<ident::foo>();
v.ensure<ident::bar>();
v.patchIfSet<ident::foo>(StringPatch::createAppend("_"));
v.patchIfSet<ident::bar>(BoolPatch{});  // no-op since inverted twice
```

### Type Alias for Patch Type

You can get the Patch type from the original type and vice versa. e.g.,

```cpp
static_assert(std::is_same_v<apache::thrift::op::patch_type<MyStruct>, MyStructPatch>);
static_assert(std::is_same_v<MyStruct, MyStructPatch::value_type>);
```

### Debugging

You can use [`apache::thrift::op::prettyPrintPatch`](https://github.com/facebook/fbthrift/blob/main/thrift/lib/cpp2/op/Patch.h#L118-L125) to pretty-print Thrift Patch for debugging purposes.


## C++ Dynamic Patch

If you can not include the generated Thrift header, you can still serialize an abritrary patch to `protocol::Object` and apply it. Here is an example for how to apply a dynamic patch

```cpp
MyStruct s;
s.field() = "hi";

MyStructPatch patch;
op::StringPatch& stringPatch = patch.patch<ident::field>();
stringPatch.prepend("(");
stringPatch.append(")");

protocol::Object dynamicStruct = apache::thrift::protocol::toObject(s);
protocol::Object dynamicPatch = patch.toObject();
protocol::applyPatch(dynamicPatch, dynamicStruct);

EXPECT_EQ(dynamicStruct[FieldId{1}].as_string(), "(hi)");

```

You can find all dynamic patch APIs [here](../../../ref/cpp/file/thrift/lib/cpp2/protocol/Patch.h).
