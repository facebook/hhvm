---
state: draft
---

# Annotations

## Structured Annotations

Structured annotations are structural constants attached to the named IDL entities, typically representing some metadata or altering the compiler behavior. They are specified via `@Annotation{key1=value1, key2=value2, ...}` syntax before the [Definitions](/idl/index.md#definitions). Each definition may have several annotations with no duplicates allowed. `@Annotation` can be used as a syntactic sugar for `@Annotation{}`.

### Grammar

```
StructuredAnnotations ::= NonEmptyStructuredAnnotationList | ""

NonEmptyStructuredAnnotationList ::=
    NonEmptyStructuredAnnotationList StructuredAnnotation
  | StructuredAnnotation

StructuredAnnotation ::= "@" ConstStruct | "@" ConstStructType
```

### Examples

Here's an example of various annotated entities.

```
struct FirstAnnotation {
  1: string name;
  2: i64 count = 1;
}

struct SecondAnnotation {
  2: i64 total = 0;
  @thrift.Box
  3: optional SecondAnnotation recurse;
  4: bool is_cool;
}

@FirstAnnotation{name="my_type"}
typedef string AnnotatedString

@FirstAnnotation{name="my_struct", count=3}
@SecondAnnotation
struct MyStruct {
  @SecondAnnotation{}
  5: AnnotatedString tag;
}

@FirstAnnotationexception
MyException {
  1: string message;
}

@SecondAnnotation{total=1, is_cool=true}
union MyUnion {
  1: i64 int_value;
  2: string string_value;
}

@SecondAnnotation{total=4, recurse=SecondAnnotation{total=5}}
service MyService {
  @SecondAnnotation
  i64 my_function(2: AnnotatedString param);
}

@FirstAnnotation{name="shiny"}
enum MyEnum {
  UNKNOWN = 0;
  @SecondAnnotation
  FIRST = 1;
}

@FirstAnnotation{name="my_hack_enum"}
const map<string, string> MyConst = {
  "ENUMERATOR": "value",
}
```

## Unstructured Annotations (Deprecated)

Unstructured annotations are key-value pairs where the key is an identifier and the value is either a string or an identifier. Both identifiers are interpreted as strings. These annotations may be applied to [definitions](/idl/index.md#definitions) in the Thrift language, following that construct.

```
Annotations ::=
  "(" AnnotationList ["," | ";"] ")" |
  "(" ")"

AnnotationList ::=
  AnnotationList ("," | ";") Annotation |
  Annotation

Annotation ::=
  Name [ "=" ( Name | StringLiteral ) ]
```

If a value is not present, then the default value of `"1"` (a string) is assumed.

## Standard Annotations

The standard Thrift annotation library is a set of structured annotations (i.e. `cpp.thrift`, `meta.thrift`, and more) that is located in `thrift/annotation`.

### Scope Annotations

How to specify what types of definitions an annotation can be applied to. See [scope.thrift](https://github.com/facebook/fbthrift/blob/v2022.07.18.00/thrift/annotation/scope.thrift#L24-L57).

### Transitive Annotations

Applies effects of sibling annotations. See [scope.thrift](https://github.com/facebook/fbthrift/blob/v2022.07.18.00/thrift/annotation/scope.thrift#L71-L94).

### Thrift annotations

Thrift annotations work in all officially supported languages.

#### thrift.Box

* Where to use: optional field name
* Value: none
* Example:

```
include "thrift/annotation/thrift.thrift"

struct RecList {
  @thrift.Box
  1: optional RecList next;
}
```

This indicates that a subobject should be allocated separately (e.g. because it is large and infrequently set).

NOTE: The APIs and initialization behavior are same as normal field, but different from `@cpp.Ref`. e.g.

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
EXPECT_FALSE(foo.normal().has_value()); // okay
EXPECT_FALSE(foo.boxed().has_value()); // okay
EXPECT_FALSE(foo.referred().has_value()); // build failure: std::unique_ptr doesn't have has_value method

EXPECT_EQ(*foo.normal(), 0); // throw bad_field_access exception
EXPECT_EQ(*foo.boxed(), 0); // throw bad_field_access exception
EXPECT_EQ(*foo.referred(), 0); // okay, field has value by default
```

#### thrift.SerializeInFieldIdOrder

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

### C++ annotations

#### cpp.type

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
This completely replaces the underlying type of a thrift for a custom implementation. It is also possible to add `cpp_include` to bring in additional data structures and use them in conjunction with `cpp.type`. It is required that the custom type matches the specified Thrift type even for internal container types. Prefer types that can leverage`reserve(size_t)` as Thrift makes uses these optimizations.*Special Case*: `cpp.type="std::unique_ptr<folly::IOBuf>"`: This annotation can be used to define a type as `IOBuf` or `unique_ptr<IOBuf>` so that you can leverage Thrift2's support for zero-copy buffer manipulation through `IOBuf`.During deserialization, thrift receives a buffer that is used to allocate the appropriate fields in the struct. When using smart pointers, instead of making a copy of the data, it only modifies the pointer to point to the address that is used by the buffer.

The custom type must provide the following methods

* *`list`*: *`push_back(T)`*
* *`map`*: *`insert(std::pair<T1, T2>)`*
* *`set`*: *`insert(T)`*

#### cpp.template

* Where to use: container field type
* Value: a template
* Example:

```
cpp_include "folly/sorted_vector_types.h"

struct Example {
  1: set<i64> (cpp.template = "folly::sorted_vector_set") shards;
}
```

Specifies a template to use for the container type to replace the default. It somewhat overlaps with `cpp.type` but the advantage is probably you just need to specify the template type once in the annotation, without repeating the inner types.

#### cpp.Ref

* Where to use: field name
* Type:
  * `cpp.RefType.Unique` : `std::unique_ptr<T>`
  * `cpp.RefType.Shared` : `std::shared_ptr<const T> `
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

Makes a field a reference, not a value and Thrift generates a `std::unique_ptr/std::shared_ptr` for the annotated field, not a value. This annotation is added to support recursive types. However, you can also use it to turn a field from a value to a pointer. `@cpp.Ref` is equivalent having type`@cpp.RefType.Unique`. All `@cpp.Ref` fields **should be** optional.

This annotation should be used instead of deprecated `cpp.ref` and `cpp.ref_type` annotations.

NOTE: A struct may transitively contain itself as a field only if at least one of the fields in the inclusion chain is either an optional Ref field or a container. Otherwise the struct would have infinite size. See [`thrift/test/Recursive.thrift`](https://github.com/facebook/fbthrift/blob/main/thrift/test/Recursive.thrift) for examples.

#### cpp.noncopyable and cpp.noncomparable

* Where to use: struct/union/exception
* Value: none
* Example:

```
typedef binary (cpp.type = "folly::IOBuf") IOBuf

union NonCopyableUnion {
  1: i32 a,
  2: IOBuf buf,
} (cpp.noncopyable, cpp.noncomparable)
```

This is to avoid generating copy constructor/copy assignment constructor and overridden equality operator for types. You rarely need to use them.

#### cpp.declare_hash and cpp.declare_equal_to

* Where to use: struct
* Value: none

Adds a `std::hash` and `std::equal_to` specialization for the Thrift struct. You need to provide your own hash and equal_to implementation. Use this annotation if you want to use your Thrift struct as the key type of `std::unordered_map` or other unordered associative containers.

#### cpp.cache

* Where to use: method parameter, must be a string and only one of the parameters can have this annotation
* Value: none

This is used in SMC to support a custom processor which caches response and uses the parameter annotated with `cpp.cache` as the key.


#### priority

* Where to use: method
* Value: none
* Example:

```
service MyService {
  void m1() (priority = HIGH);
  void m2();
}
```

Designate a priority level for the annotated method. By default all methods have the same priority level. This is only used in C++ when `ThriftServer` uses a `PriorityThreadManager`.

#### message

* Where to use: exception
* Value: name of a string field
* Example:

```
exception MyException {
  1: string errorMessage
} (message = "errorMessage")
```

The annotation's value should be the name of a string field in the exception. The code generator then generates a constructor which uses a string to initialize that string field. The exception class's `what()` method is overridden to returns the content of that string field.

#### no_default_comparators

* Where to use: struct
* Value: none

Do not generate a default comparator even if the struct is orderable.

#### thread

* Where to use: method
* Value: eb or tm (default)
* Example:

```
service MyService {
  int m1() (thread = 'eb');
  int m2();
}
```

Causes the request to be executed on the event base thread directly instead of rescheduling onto a thread manager thread, provided the async_eb_ handler method is implemented. You should only execute the request on the event base thread if it is very fast and you have measured that rescheduling is a substantial chunk of your service's CPU usage. If a request executing on the event base thread blocks or takes a long time, all other requests sharing the same event base are affected and latency will increase significantly. This is only supported in C++. We strongly discourage the use of this annotation unless strictly necessary. You will have to implement the harder-to-use async_eb_ handler method; implementing any other method will cause the rescheduling regardless of this annotation. This also disables queue timeouts, an important form of overload protection.

#### cpp.indirection

* Where to use: field type
* Value: a method to call from the base type
* Example:

```
# Thrift file
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

Together with `cpp.type` annotation, this annotation defines a new type and uses the indirect method to access its value.

#### frozen and frozen2

* Where to use: struct, not exception. This is also supported through code generator flag and currently only used through code generator flag.

#### cpp.methods (deprecated)

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

Insert some code into the Thrift generated header file.

#### cpp.EnumType

* Where to use: enum
* Type: an integer type for c++ to use as the underlying type of enum
  * `cpp.EnumUnderlyingType.I8` : `std::int8_t`
  * `cpp.EnumUnderlyingType.U8` : `std::uint8_t`
  * `cpp.EnumUnderlyingType.I16` : `std::int16_t`
  * `cpp.EnumUnderlyingType.U16` : `std::uint16_t`
  * `cpp.EnumUnderlyingType.U32` : `std::uint32_t`:
* Example:


```
@cpp.EnumType{type = cpp.EnumUnderlyingType.I8}
enum TermIdType {
 GENERIC_FP96 = 0,
 PREFIX_USERID = 1,
}
```
NOTE: 64-bit integer and signed 32-bit integer are not valid options. Signed 32-bit integer is the default and 64-bit is not supported to avoid truncation since enums are sent as 32-bit integers over the wire.

#### deprecated

* Where to use: struct/field
* Value: none or a custom message

This will mark a struct or a field as deprecated and will issue a warning when compiling. Note that if other Thrift files include a Thrift file with deprecated annotations, those files will be treated as using these fields and propagate warnings when included. If `a.thrift` marks struct `foo` as deprecated, `b.thrift` includes `a.thrift`, and `c.cpp` includes `b.thrift`, deprecated warnings concerning foo will be raised when building `c.cpp` even if the file doesn't actually use `foo`.

* Example:

```
// Default struct message: "class MyStruct is deprecated"
struct MyStruct {
 1: i32 a
} (deprecated)

struct MyStruct3 {
 1: i32 a = 0 (deprecated)  // Default message: "i32 a is deprecated"
} (deprecated = "This is not longer supported")
```

#### cpp.MinimizePadding

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

which gives the size of 16 bytes compared to 32 bytes if `cpp.MinimizePadding` was not specified.

#### cpp.name

* Where to use: field
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

#### cpp.mixin

* Where to use: field
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

Read more: Thrift/Mixins

#### cpp.Lazy

* Where to use: field
* Value: none
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

Read more: Thrift/Lazy/

#### cpp.PackIsset

* Where to use: field
* Value: none
* Example:

```
include "thrift/annotation/cpp.thrift"

@cpp.PackIsset
struct Foo {
  1: optional i32 field1
  2: i32 field2
}
```
Read more: Thrift/Isset_Bitpacking/

#### cpp.Adapter

* Where to use: field, typedef
* Value: name of adapter
* Example:

```
// In a C++ header:
struct BitsetAdapter {
  static std::bitset<32> fromThrift(std::uint32_t t) { return {t}; }
  static std::uint32_t toThrift(std::bitset<32> t) { return t.to_ullong(); }
};

// In a thrift file:
struct Foo {
  @cpp.Adapter{name = "BitsetAdapter"}
  1: i32 flags;
}

// In a C++ source file:
Foo foo;
foo.flags()->set(0); // set 0th bit
```

More detail: Thrift/Thrift_Guide/Adapter/

#### Annotations for C++ Allocator Awareness

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

#### cpp.allocator

* Where to use: struct
* Value: string name of a type conforming to the [C++ named requirement "Allocator"](https://en.cppreference.com/w/cpp/named_req/Allocator)

Declares that this is an allocator-aware structure, using the specified C++ allocator type. When this annotation is present, the Thrift compiler will generate additional code sufficient to meet the [C++ named requirement "AllocatorAwareContainer":](https://en.cppreference.com/w/cpp/named_req/AllocatorAwareContainer)

* `allocator_type` typedef
* `get_allocator()` function
* Three "allocator-extended" constructors

#### cpp.use_allocator

* Where to use: field
* Value: none

Indicates that the structure's allocator should be passed down to this field at construction time.

#### cpp.allocator_via

* Where to use: struct
* Value: string with a name of a field in this struct

This is a structure size optimization. The naive implementation of `get_allocator()` requires an additional data member to “remember” the allocator. With `cpp.allocator_via`, we instead delegate this responsibility to one of the allocator-aware fields.

### Hack annotations

#### hack.adapter

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

#### hack.attributes

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

### Python annotations

#### python.Adapter

* Where to use: field, typedef
* Value: a class that implements the [Adapter](https://github.com/facebook/fbthrift/blob/main/thrift/lib/python/adapter.pyi)
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

<FbInternalOnly>

See [Adapters](/features/adapters.md) for more detail.

</FbInternalOnly>
