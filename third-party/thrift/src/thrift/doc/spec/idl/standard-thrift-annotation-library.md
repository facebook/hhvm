# Using Annotations

<!-- https://www.internalfb.com/intern/wiki/Thrift/Thrift_Guide/IDL/Standard_Thrift_Annotation_Library/?noredirect -->

:::note

Unstructured annotations will be replaced with structured annotations. Please use structured annotations when possible.

:::

## Standard Thrift Annotation Library

The standard Thrift annotation library is a set of structured annotations (i.e. `cpp.thrift`, `meta.thrift`, and more) that is located in `thrift/annotation`.

The standard Thrift annotation library exists in the following path for each repository:

* fbsource
   * `fbcode/thrift/annotation`
* configerator
   * `/usr/facebook/thrift/annotation`

## C++ annotations

For C++, the code generator itself ([thrift/compiler/generate/t_mstch_cpp2_generator.cc](https://l.facebook.com/l.php?u=https%3A%2F%2Fphabricator.fb.com%2Fdiffusion%2FFBCODE%2Fbrowse%2Fmaster%2Fthrift%2Fcompiler%2Fgenerate%2Ft_mstch_cpp2_generator.cc&h=AT2y3p5JguTzt_CNFLH3TlTUq9t-nC5UcWo-RIt3syvqh7I9EG0JvkUpSdiRfQw8e9YkVIDZ3P1n5QxYDEYkdVacxP5l2XAySu0msoCko2gvrEvzuxgZ87CxAEGY225PCD1ZZpFbyjWJJWek7Ha87925fGY)) is the real source of truth but it will be hard to find the exact meaning of each annotation, partially because these annotations are scattered in the code generator. We will try to make this wiki up-to-date but when in doubt, consult the thrift team or check the code generator directly.

### cpp vs cpp2

Annotation sometimes are prefixed by cpp/cpp2, for example `cpp.ref` or `cpp2.type`. Annotation not prefixed by cpp/cpp2 aren't very different from the ones prefixed by cpp/cpp2. The only difference might be these annotations can be used by code generators of other languages, but unfortunately some of them are actually only for cpp/cpp2.

**Warning**: unrecognized unstructured annotations are *silently ignored*. So, for example, passing `cpp2.adapter` does absolutely nothing (the correct syntax is `cpp.adapter`). This is not a problem for structured annotation.

### cpp.type

* Where to use: field type
* Value: a complete type
* Example:

```
cpp_include "<unordered_map>"
cpp_include "folly/sorted_vector_types.h"
cpp_include "CustomMap.h"

typedef list<i64> Type1 (cpp.type = "folly::small_vector<int64_t>")
typedef map<string, Foo> Type2 (cpp.type = "std::unordered_map<std::string, Foo>")
typedef binary (cpp.type = "std::unique_ptr<folly::IOBuf>") IOBufPtr

struct Foo {
  1: Type1 field1
  2: Type2 field2
  3: map<i64, double> (cpp.type = "CustomMap") field3
  4: IOBufPtr iobuf_ptr
}
```
This completely replaces the underlying type of a thrift for a custom implementation. It is also possible to add `cpp_include` to bring in additional data structures and use them in conjunction with `cpp2.type`. It is required that the custom type matches the specified Thrift type even for internal container types. Prefer types that can leverage`reserve(size_t)` as Thrift makes uses these optimizations.*Special Case*: `cpp.type="std::unique_ptr<folly::IOBuf>"`: This annotation can be used to define a type as `IOBuf` or `unique_ptr<IOBuf>` so that you can leverage Thrift2's support for zero-copy buffer manipulation through `IOBuf`.During deserialization, thrift receives a buffer that is used to allocate the appropriate fields in the struct. When using smart pointers, instead of making a copy of the data, it only modifies the pointer to point to the address that is used by the buffer.**Warning: Thrift assumes that your** **`cpp.type`** **supports:**

* *`list`*: ***`push_back(T)`*
* *`map`*: ***`insert(std::pair<T1, T2>)`*
* *`set`*: ***`insert(T)`*

### cpp.template

* Where to use: container field type
* Value: a template
* Example:

```
cpp_include "folly/sorted_vector_types.h"  // Inside some thrift structs
  1: set<i64> (cpp.template = "folly::sorted_vector_set") shards;
```
Specifies a template to use for the container type to replace the default. It somewhat overlaps with `cpp.type` but the advantage is probably you just need to specify the template type once in the annotation, without repeating the inner types.

### thrift.Box

* Where to use: field name
* Value: `true`
* Example:

```
struct RecList {
  @thrift.Box
  1: optional RecList next;
}
```
Similar to @cpp.Ref that thrift generates std::unique_ptr for the annotated field. The API is almost equivalent to a regular thrift field. @cpp.Ref annotation with std::unique_ptr is deprecated. NOTE: The initialization behavior is same as normal field, but different from @cpp.Ref. e.g.

```
struct Foo {
  1: optional i32 normal;
  @thrift.Box
  2: optional i32 boxed;
  @cpp.Ref
  3: optional i32 referred;
}
```
in C++

```
Foo foo;
EXPECT_EQ(*foo.normal(), 0); // error: field doesn't have value.
EXPECT_EQ(*foo.boxed(), 0); // error: field doesn't have value.
EXPECT_EQ(*foo.referred(), 0); // okay, field has value by default
```
### cpp.Ref

* Where to use: field name
* Type:
   * `cpp.RefType.Unique` : `std::unique_ptr<T>`
   * `cpp.RefType.Shared` : `std::shared_ptr<const T>`
   * `cpp.RefType.SharedMutable` : `std::shared_ptr<T>`:
* Example:

```
struct BinaryTree {
  @cpp.Ref{type = cpp.RefType.Unique}
  1: BinaryTree left;
  @cpp.Ref
  2: BinaryTree right;
  @cpp.Ref{type = cpp.RefType.SharedMutable}
  3: BinaryTree parent;
  @cpp.Ref{type = cpp.RefType.Shared}
  4: BinaryTree root;
  5: i16 item;
}
```
Makes a field a reference, not a value and Thrift generates a `std::unique_ptr/std::shared_ptr` for the annotated field, not a value.This annotation is added to support recursive types. However, you can also use it to turn a field from a value to a pointer. Note if it is a container type, you don't need this annotation. You still need this if your struct indirectly contains a member of itself. See https://phabricator.fb.com/diffusion/FBCODE/browse/master/thrift/test/Recursive.thrift for examples.Also if you use it to support recursive types, be sure to make the field optional. The reason is as a recursive field, at some point you must assign a `nullptr` to it to stop the recursion. It obviously cannot be required. If it is default (not optional or required), Thrift always serializes an empty struct and this is different from a `nullptr` and it fails serialization/deserialization round trip equality test. For more information see the discussion in [D2145563](https://www.internalfb.com/intern/diff/2145563/) and we also added a Thrift compiler warning: "cpp.ref field must be optional if it is recursive".`@cpp.Ref` is equivalent having type`@cpp.RefType.Unique`.

### cpp2.noncopyable and cpp2.noncomparable

* Where to use: struct/union/exception
* Value: None
* Example:

```
typedef binary (cpp.type = "folly::IOBuf") IOBuf

union NonCopyableUnion {
  1: i32 a,
  2: IOBuf buf,
} (cpp2.noncopyable, cpp2.noncomparable)
```
This is to avoid generating copy constructor/copy assignment constructor and overridden equality operator for types. You rarely need to use them.

### cpp.declare_hash and cpp.declare_equal_to

* Where to use: struct
* Value: None

Adds a `std::hash` and `std::equal_to` specialization for the Thrift struct. You need to provide your own hash and equal_to implementation. Use this annotation if you want to use your Thrift struct as the key type of `std::unordered_map` or other unordered associative containers.

### cpp.cache

* Where to use: method parameter, must be a string and only one of the parameters can have this annotation
* Value: None

This is added by the SMC team to support their custom processor, which caches response and uses the parameter annotated by `cpp.cache` as the key.

### cpp.coroutine

* Where to use: method
* Value: None
* Example:

```
service MyService {
  void m1() (cpp.coroutine);
}
```
Enable coroutine generation. See https://www.internalfb.com/intern/wiki/Thrift/ImplementingAServer/ for more information.

### priority

* Where to use: method
* Value: None
* Example:

```
service MyService {
  void m1() (priority = HIGH);
  void m2();
}
```
Designate a priority level for the annotated method. By default all methods have the same priority level. This is only used in cpp2 when `ThriftServer` uses a `PriorityThreadManager`.

### message

* Where to use: exception
* Value: name of a string field
* Example:

```
exception MyException {
  1: string errorMessage
} (message = "errorMessage")
```
The annotation's value should be the name of a string field in the exception. The code generator then generates a constructor which uses a string to initialize that string field. The exception class's `what()` method is overridden to returns the content of that string field.

### final

* Where to use: struct
* Value: none

Do not allow user classes inheriting from a thrift struct.

### no_default_comparators

* Where to use: struct
* Value: none

Do not generate a default comparator even if the struct is orderable. This is only supported in cpp2 and there is no usage in fbcode, so maybe another test feature.

### thread

* Where to use: method
* Value: eb or tm (default)
* Example:

```
service MyService {
  int m1() (thread = 'eb');
  int m2();
}
```
Causes the request to be executed on the event base thread directly instead of rescheduling onto a thread manager thread, provided the async_eb_ handler method is implemented. You should only execute the request on the event base thread if it is very fast and you have measured that rescheduling is a substantial chunk of your service's CPU usage. If a request executing on the event base thread blocks or takes a long time, all other requests sharing the same event base are affected and latency will increase significantly. This is only supported in cpp2.We strongly discourage the use of this annotation unless strictly necessary. You will have to implement the harder-to-use async_eb_ handler method; implementing any other method will cause the rescheduling regardless of this annotation. This also disables queue timeouts, an important form of overload protection.

### cpp.indirection

* Where to use: field type
* Value: a method to call from the base type
* Example:

```
## Thrift file
typedef i64 (cpp.type = "Seconds", cpp.indirection) seconds
struct MyStruct {
  1: seconds s;
}
// C++ file

struct Seconds : private boost::totally_ordered<Seconds> {
  FBTHRIFT_CPP_DEFINE_MEMBER_INDIRECTION_FN(number);
  std::int64_t number = 0;
  Seconds() = default;
  Seconds(std::int64_t number_) : number(number_) {}
  bool operator==(Seconds that) const { return number == that.number; }
  bool operator<(Seconds that) const { return number < that.number; }
};
```
Together with `cpp.type` annotation, this annotation defines a new type and uses the indirect method to access its value. Only supported in cpp.

### frozen and frozen2

* Where to use: struct, not exception. This is also supported through code generator flag and currently only used through code generator flag.

### cpp.methods

* Where to use: struct
* Value: valid code
* Example:

```
struct MyStruct {
    1: i32 anInteger;
  } (
    cpp.methods = "
      public:
        std::string toString() const;
    "
  )
```
Insert some code into the Thrift generated header file. Works in both cpp and cpp2. Unsupported.

### cpp.EnumType

* Where to use: enum
* Value: `cpp.EnumUnderlyingType.{U8, I8, U16, I16, U32}`
* Example:

```
@cpp.EnumType{type=cpp.EnumUnderlyingType.U8}
enum TermIdType {
 GENERIC_FP96 = 0,
 PREFIX_USERID = 1,
}
```
There is no way to expliclity specify I32, as this is the default. There is no way to set an enum's underlying type to a 64-bit integer (either signed or unsigned) because all Thrift enums are serialized to 32 bit integers and thus would truncate larger values during serialization and deserialization.

### deprecated

* Where to use: struct/member
* Value: None or a custom message.

This will mark the struct or member as deprecated and will issue a warning when compiling.Note that if other Thrift files include a Thrift file with deprecated annotations, those files will be treated as using these fields and propagate warnings when included. If `a.thrift` marks struct `foo` as deprecated, `b.thrift` includes `a.thrift`, and `c.cpp` includes `b.thrift`, deprecated warnings concerning foo will be raised when building `c.cpp` even if the file doesn't actually use `foo`.

* Example:

```
struct MyStruct {
 1: i32 a
} (deprecated)// Default struct message: "class MyStruct is deprecated"struct MyStruct3 {
 1: i32 a = 0 (deprecated)
} (deprecated = "This is not longer supported") // Default member message: "i32 a is deprecated"
```
### cpp.MinimizePadding

* Where to use: struct
* Value: none
* Example:

```
include "thrift/annotation/cpp.thrift"

@cpp.MinimizePadding
struct Padded {
  1: required byte small
  2: required i64 big
  3: required i16 medium
  4: required i32 biggish
  5: required byte tiny
}
```
This annotation enables reordering of fields in the generated C++ struct to minimize padding. This is achieved by placing the fields in the order of decreasing alignments. The order of fields with the same alignment is preserved.For example, the C++ fields for the `Padded` Thrift struct above will be generated in the following order:

```
int64_t big;
int32_t biggish;
int16_t medium;
int8_t small;
int8_t tiny;
```
which gives the size of 16 bytes compared to 32 bytes if `cpp.minimize_padding` was not specified.

### cpp.name

* Where to use: member
* Value: string with a valid C++ identifier
* Example:

```
struct Error {
  1: string message
  2: i32 errno (cpp.name = "error_code")
}
```
This annotation renames all C++ entities (member variables, parameter names, etc.) generated from the Thrift field. For example, the `errno` field in the `Error` struct will be renamed to `error_code`:

```
class Error {
 public:
  Error() : error_code(0) {}

  std::string message;
  int32_t error_code;
};
```
In most cases a much better solution is to rename the problematic Thrift field itself. Only use the `cpp.name` annotation if such renaming is problematic, e.g. when the field name appears in code as a string, particularly when using JSON serialization, and it is hard to change all usage sites.

### cpp.mixin

* Where to use: member
* Value: none
* Example:

```
struct Mixin1 { 1: i32 field1 }
struct Mixin2 { 1: i32 field2 }
struct Foo {
  1: Mixin1 m1 (cpp.mixin);
  2: Mixin2 m2 (cpp.mixin);
  3: i32 field3;
}
```
Then in C++ we could access fields in Mixin from Foo

```
Foo f;
f.field1_ref();  // Access field1 in Mixin1
```
[Read more](https://www.internalfb.com/intern/wiki/Thrift/Mixins/)

### cpp.experimental.lazy

* Where to use: field
* Value: `true`
* Example:

```
struct Foo {
  1: list<i32> small_field;
  2: list<i32> large_field (cpp.experimental.lazy);
}
```
Lazily deserialize large field on first access.

```
Foo foo;
apache::thrift::CompactSerializer::deserialize(serializedData, foo);

// large_field is lazy field, it will be deserialized on first access
// The data will be deserialized in method call large_field_ref()
LOG(INFO) << foo.large_field_ref()->size();

// Result will be cached, we won't deserialize again
LOG(INFO) << foo.large_field_ref()->size();
```
[Read more](https://www.internalfb.com/intern/wiki/Thrift/Lazy/)

### `cpp.PackIsset`

* Where to use: field
* Value: `none`
* Example:

```
include "thrift/annotation/cpp.thrift"
@cpp.PackIsset
struct Foo {
  1: optional i32 field1
  2: i32 field2
}
```
[Read more](https://www.internalfb.com/intern/wiki/Thrift/Isset_Bitpacking/)

### `cpp.Adapter`

* Where to use: field, typedef
* Value: name of adapter
* Example:

```
// in C++ Header
struct BitsetAdapter {
  static std::bitset<32> fromThrift(std::uint32_t t) { return {t}; }
  static std::uint32_t toThrift(std::bitset<32> t) { return t.to_ullong(); }
};

// In thrift file
struct Foo {
  @cpp.Adapter{name = "BitsetAdapter"}
  1: i32 flags;
}

// in C++ file
Foo foo;
foo.flags()->set(0); // set 0th bit
```
More detail: [Adapters](/fb/features/adapters.md)

## Annotations for C++ Allocator Awareness

Example C++:

```
using MyAlloc = std::scoped_allocator_adaptor<folly::SysArenaAllocator<char>>;
template <class T> using MyVector = std::vector<T, MyAlloc>;
template <class K, class V> using MyMap = std::map<K, V, std::less<K>, MyAlloc>;
```
Thrift IDL usage example (assuming `cpp_include` the above):

```
struct aa_struct {
  1: list<i32> (cpp.use_allocator, cpp.template = "MyVector") aa_list;
  2: map<i32, i32> (cpp.use_allocator, cpp.template = "MyMap") aa_map;
  3: set<i32> not_aa_set;
} (cpp.allocator = "MyAlloc", cpp.allocator_via = "aa_list")
```
### cpp.allocator

* Where to use: struct
* Value: string name of a type conforming to the [C++ named requirement "Allocator"](https://l.facebook.com/l.php?u=https%3A%2F%2Fen.cppreference.com%2Fw%2Fcpp%2Fnamed_req%2FAllocator&h=AT2nZRST6TZdM_bcQzeJfxXpvh3V9nr_DvlFj21QU1vVT297eKYDLoukR4e95qUAXhNHFqDaRvGJfw3Bf5kNZ_ZXQHRAFyBIHj7ivPr13-QrrX_QUAdhsTZ8ReE60qHiE44F_7B2sf3XRtCx2Chi9hR65W3H2h2j6k9xIPKh)

Declares that this is an allocator-aware structure, using the specified C++ allocator type. When this annotation is present, the Thrift compiler will generate additional code sufficient to meet the [C++ named requirement "AllocatorAwareContainer":](https://l.facebook.com/l.php?u=https%3A%2F%2Fen.cppreference.com%2Fw%2Fcpp%2Fnamed_req%2FAllocatorAwareContainer&h=AT2bRfsmx8T-pt5eoFPlkyFypwos-UW_HhqqsgcFY56-0N4aZzvDYnCIMe5Y5mxfV_zIdMgqNdnPgi0RV0TrZV2sLRrmoREtbtIaVhRYcy2MPWvpBclUuebOyA_4j3IEWinffeCGyshL3QAPLi7W_IIw0iQ)

* `allocator_type` typedef
* `get_allocator()` function
* Three "allocator-extended" constructors

### cpp.use_allocator

* Where to use: member
* Value: none

Indicates that the structure's allocator should be passed down to this member at construction time.

### cpp.allocator_via

* Where to use: struct
* Value: string with a valid C++ identifier which is a member field in this struct

This is a structure size optimization. The naive implementation of `get_allocator()` requires an additional data member to “remember” the allocator. With `cpp.allocator_via`, we instead delegate this responsibility to one of the allocator-aware fields.

## Hack annotations

### hack.adapter

* Where to use: field type
* Value: a class that implements the `IThriftAdapter` interface
* Example:

```
// in thrift:
struct Document {
  1: i32 (hack.adapter = '\TimestampToTimeAdapter') created_time;
}
// thrift compiler will generate this for you:
class Document implements \IThriftStruct {
...
  public Time $created_time;
...
}
// in hack:
final class TimestampToTimeAdapter implements IThriftAdapter {
  const type TThriftType = int;
  const type THackType = Time;
  public static function fromThrift(int $seconds)[]: Time {
    return Time::fromEpochSeconds($seconds);
  }
  public static function toThrift(Time $time): int {
    return $hack_value->asFullSecondsSinceEpoch();
  }
}
// elsewhere in hack:
function timeSinceCreated(Document $doc): Duration {
  // $doc->created_time is of type Time
  return Duration::between(Time::now(), $doc->created_time);
}
```
This completely replaces the underlying type of a thrift for a custom implementation and uses the specified adapter to convert to and from the underlying Thrift type during (de)serialization.

More detail: https://www.internalfb.com/intern/wiki/Thrift/Thrift_Guide/Adapter/

### hack.attributes

* Where to use: field or struct type
* Value: add attributes like `JSEnum` to structs or fields
* Example:

```
// In thrift
enum MyEnum {
  ALLOWED = 1,
  THIS_IS_ALLOWED  =  2,
  THIS_IS_ALLOWED_2 = 3,
}(
  hack.attributes=
    "\JSEnum(shape('name' => 'MyEnum')),
    \GraphQLEnum('MyEnum', 'Description for my enum',)"
)
struct MyThriftStruct {
  1: string foo (hack.attributes = "FieldAttribute");
  2: string bar;
  3: string baz;
} (hack.attributes = "ClassAttribute")
```
```
//thrift compiler will generate this for you
<<\JSEnum(shape('name' => 'MyEnum')),
\GraphQLEnum('MyEnum', 'Description for my enum',)>>
enum MyEnum: int {
 ALLOWED = 1;
 THIS_IS_ALLOWED = 2;
 THIS_IS_ALLOWED_2 = 3;
}
<<ClassAttribute>>
class MyThriftStruct implements \IThriftStruct {
 ....

 <<FieldAttribute>>
 public string $foo;

 ....
}
```

## Python annotations

### python.Adapter

* Where to use: field, typedef
* Value: a class that implements the [`Adapter`](https://www.internalfb.com/code/fbsource/[8123a79c82306b94e9085db75aaf90f34a4129b3]/fbcode/thrift/lib/python/adapters/base.py?lines=24-51)
* Example:

```
// In Python file
class DatetimeAdapter(Adapter[int, datetime]):
    @classmethod
    def from_thrift(cls, original: int) -> datetime:
        return datetime.fromtimestamp(original)

    @classmethod
    def to_thrift(cls, adapted: datetime) -> int:
        return int(adapted.timestamp())

// In thrift file
struct Foo {
  @python.Adapter{
    name = "DatetimeAdapter",
    typeHint = "datetime.datetime",
  }
  1: i32 dt;
}

// In python file
foo = Foo()
assert isinstance(foo.dt, datetime)
assert foo.dt.timestamp() == 0
```
This completely replaces the underlying type of a thrift for a custom implementation and uses the specified adapter to convert to and from the underlying Thrift type during (de)serialization.

## Thrift annotations

Thrift annotations work in all officially supported languages.

### thrift.SerializeInFieldIdOrder

Serialize fields in field id ascending order instead of fields declaration order. This makes serialization result deterministic after swapping fields. In addition, it can reduce payload size only for compact protocol. Example

```
@thrift.SerializeInFieldIdOrder
struct Foo {
  2: byte a;
  1: byte b;
}
```
This reduces serialized data size from 5 bytes to 4 bytes for compact protocol.

Why? Compact protocol stored delta of field id, instead of actual field id in payload. When field is serialized out of field id order, delta=0 will be written, followed with actual field id, thus when field is serialized out of field id order, it uses more bytes for field id.

NOTE: This annotation won't reduce payload size for other protocols.

### thrift.TerseWrite

* Where to use: field/struct/package
* Value: `true`
* Example:

```
@thrift.TerseWrite
package "standard/thrift/annotation/example"

struct Foo {
  @thrift.TerseWrite
  1: i32 a;
}

@thrift.TerseWrite
struct TerseFoo{
  1: i32 a;
}

struct RawFoo {
  1: i32 a;
}
```
The annotation promotes an unqualified field to a terse field. All `Foo.a`, `TerseFoo.a`, and `RawFoo.a` are terse fields.

## Rust annotations

Please see their [list of annotations](https://www.internalfb.com/intern/wiki/Rust-at-facebook/Thrift/IDL_Annotations/).
