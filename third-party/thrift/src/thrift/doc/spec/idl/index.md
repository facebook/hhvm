# Thrift Interface Definition Language

<!-- https://www.internalfb.com/intern/wiki/Thrift/Thrift_Guide/IDL/?noredirect -->

Thrift supports serialization and RPC across multiple languages. In order to achieve that, it allows users to define schemas for structures and interfaces for services in an IDL. In addition to going over the basics, this guide will call out some gotchas in core languages that the Thrift team supports at Facebook (hack, c++, java, and python).

Thrift schemas and interfaces are defined in `.thrift` files. The Thrift compiler consumes these files to autogenerate structures and RPC boilerplate in different languages.

---

## **Base Types**

### **Primitive Types**

* `bool`: A one bit boolean
* `byte`: A signed byte (8-bits)
* `i16`: A 16-bit signed integer
* `i32`: A 32-bit signed integer
* `i64`: A 64-bit signed integer
* `float`: A 32-bit floating point number
* `double`: A 64-bit floating point number
* `binary`: A byte array
* `string`: A UTF-8 string

> Note: Thrift does not support unsigned integers because they have no direct translation to native (primitive) types in many of Thrift’s target languages.Note: Some target languages enforce that `string` values are actually UTF-8 encoded and some target languages do not. For example: Java and Python care, while C++ does not. This can appear as cross-language incompatibility.Note: `binary` and `string` are encoded identically for RPC (Binary Protocol & Compact Protocol) and are interchangeable, but will be encoded differently in SimpleJSONProtocol & JSONProtocol: a binary field will be base64 encoded, while a string will be the same (escaped).


### **Containers**

Thrift has strongly typed containers that map to the most commonly used containers in popular programming languages. They are annotated using the Java Generics style. There are three containers types available:

* `list<t1>`: A list of elements of type t1. May contain duplicates.
* `set<t1>`: An unordered set of unique elements of type t1. Unique elements.
* `map<t1,t2>`: An unordered map of strictly unique keys of type t1 to values of type t2. Unique elements.

> Note: The default mode is to use ordered sets and maps. However, some languages allow the use of unordered and customized containers - see [Thrift Annotations](/spec/definition/annotation.md#unstructured-annotations-deprecated).

*CAUTION:* Although not enforced, it is strongly encouraged to only use set and map when t1 is either a string or an integer type for the highest compatibility between languages.

Thrift also supports nested containers.  Any container can have elements of type containers such as:

* `set<list<t1>>` , `list<map<t1,t2>>`, and `map<t1, map<t2, t3>>`.

Note: All containers must declared with `const`.

### Initialization

A base type can always be initialized with default values. These are some examples of how default values can be set.

```
// Booleans
bool field1 = true
bool field2 = false

// Integers
// Value has to fit in the amount of allocated bits
byte field3 = -10
i32 field4 = 200
i64 field5 = 3000000000
i16 field6 = 0241 // Octals (start with 0)
i32 field7 = 0xFA12EE // Hex (start with 0x)

// Floating point numbers
// Value has to fit in the amount of allocated bits
float field8 = -50.15
float field9 = 0.0
double field10 = 400.21

// Strings (double or single quotes)
string field11 = 'foo'
string field12 = "bar"
string field13 = 'foo:"bar"'
string field14 = "bar:'foo'"
```
Containers should be initialized with JSON formatting in either a single line or in multiple lines. Thrift also allows the use of trailing commas.

```
// For list and set use '[ ]'
const list<i32> AList = [2, 3, 5, 7]
const set<string> ASet = [
  "foo",
  "bar",
  "baz",
]

// For map use '{ : }'
const map<string, list<i32>> AMap = {
  "foo" : [1, 2, 3, 4],
  "bar" : [10, 32, 54],
}
```
### **Enums**

Enums allows the serialization of a pre-defined set of values. Values are represented as integers and the values should always be specified

```
enum Foo {
  Unknown = 0,
  Two = 2,
  Three = 3,
  Five = 5,
}
```
> CAUTION: an enum is default initialized with `0`, therefore we **strongly encourage** you to add an `Unknown` entry for `0` so that it is not inadvertently initialized to some meaningful value by accident.


Enums can also be used in containers.

```
struct Bar {
  list<Foo> = [Foo.Five, Foo.Two]
}
```
> *CAUTION:* Removing and adding enums values can be dangerous - see [https://www.internalfb.com/intern/wiki/Thrift/SchemaEvolution/](https://www.internalfb.com/intern/wiki/Thrift/Versioning).


## **Collections**

Every collection type element features a list of fields which are required to have a unique integer identifier. These are used by the deserialization algorithms to identify the correct element within the serialized object. Id's are the first value that every field must have.

```
struct Foo {
  1: bool fieldA,
  2: i32 fieldB,
  3: i64 fieldC,
  4: string fieldD,
}
```
It is important to keep in mind the following rules when writing fields for collections:

* Every field needs a unique positive number identifier.
* The values of valid id's range are: [1, 2^63 - 1].
* Field id are not position relevant. Meaning that fields do not necessarily need to start with 1 or have a sequential list of elements. For example, the following struct is also valid:

```
struct Foo {
  590: bool field1,
  36: i64 field2,
}
```
This gives the flexibility of rearranging the order of the fields within a Thrift file and it won't affect the serialization scheme. For example, these two structs are equivalent in every sense

```
struct Foo {
1: string field1,
2: i64 field2,
3: i64 field3,
4: double field4,
}

struct Foo {
3: i64 field3,
2: i64 field2,
4: double field4,
1: string field1,
}
```
> **WARNING:** Do not reuse ids. If a field is removed from a struct, it's a good practice to comment it out as a reminder to not to reuse it - see [Thrift Versioning](https://www.internalfb.com/intern/wiki/Thrift/Versioning).


### **Structs**

Structs are a collection of fields. Each struct field has a unique integer identifier, a type, a unique field name, a level of required-ness, and a default value. It is easiest to illustrate this concept with an example. You can use trailing comma, semicolon, or nothing at all.

```
struct Foo {
  1: i64 field1 = 101,
  2: required string field2,
  3: optional list<string> field3,
}
```
Take struct Foo as an example. Foo has three fields, each with a unique id (1, 2, and 3), a unique name (field1, field2, field3), and a type (e.g. Foo::field1 is an i64). A type can be a base type, a collection, or another struct. Thrift does not support structure inheritance.

### **Exceptions**

Exceptions are created in the exact same way as a struct. These help to add useful messages to service functions that can throw exceptions.

```
exception Foo {
  1: i32 errorCode,
  2: string message,
} (message = "message") // optional, tells ServiceRouter which field contains the message
```
### **Unions**

Unions are similar to structs but can only have at most one field set at a time.

```
// Unions only allow Foo to have field1, field2, or field3 set.
union Foo {
  1: string field1,
  2: i64 field2,
  3: list<string> field3,
}
```
In addition to enforcing the union semantics during serialization and deserialization, union structs are more space efficient in some languages.

### **Intrinsic Default Values**

If a struct doesn't provide a value to a field, they will be serialized and deserialized with the following default values.

* bool: false
* byte or integers: 0
* enum: 0 (even if the enum has no value for zero)
* float or double: 0.0
* string or binary: 0 length
* containers: Empty container

### **Pre-defined value**

It is also possible to define a custom default value for a struct field.

If this field is not serialized with any data, it will be serialized with the provided values. (until thrift-v1).

```
enum Foo { A = 1, B = 2, C = 3}
struct Bar {
  1: i64 field1 = 10,
  2: i64 field2,
  3: map<i32, string> field3 = {15 : 'a_value', 2: 'b_value'},
  4: list<Foo> = [Foo.A, Foo.B, Foo.C],
}
```
> *CAUTION:* Avoid using default values on optional fields. It is not possible to tell if the server sent the value, or if it is just the default value.


> **WARNING:** Do not change default values after setting them. It is hard to know what code may be relying on the previous default values.


> **WARNING:** Default value relies on default constructor in C++ (see [discussion here](https://fburl.com/6vccs6jw)). Creating the thrift struct with custom constructor (i.e. via `cpp.methods`) will make default value useless. Make sure to initialize primitive types in custom constructor as always, otherwise those variables will not be initialized.


### **Struct Modifiers**

Each field may be marked as required or optional. Fields can also be *set *or *unset (*contains or doesn't contain a value). The following specifications determine the serialization and deserialization behaviors of fields.

* Default (No modifier):
   * The field is added to the serialized structure, even if the field is unset.
   * The field is left unchanged when deserializing a structure if the field isn't present in the serialized data.
   * Will cause undefined behavior if the field is null when serializing a structure. (We are working on defining the appropriate behavior).
* `optional`:
   * The field is added to the serializing a structure only if the field is set in the structure.
   * The field is left unchanged when deserializing a structure if the field isn't present in the serialized data.
* `required` (2019-07-09 do not use, it is going to be removed):
   * The field is always added to the serialized structure.
   * The field has to be set, otherwise an exception will be thrown if the field is not present when deserializing.
   * Causes undefined behavior if the field is null when serializing a structure. (We are working on defining the appropriate behavior).

For default and optional fields, a missing field will be marked as unset when it is deserialized. It is possible to provide a default value to a field. If a field has a default value, instead of marking a missing field as unset when deserializing, it will instead be given the default value. Default values should be written in JSON notation.

> **WARNING:** Marking a field as required makes it difficult to remove it from the struct later. It is required to ensure that all users of serialization are “downgraded” to unmarked, and are deployed, before removing the field. When used in RPC, this may involve coordinating deployments of clients and servers. Generally, it's not a practice to use required.


---

## **Constants**

Constants can be specified using the const keyword. These can be used as a default values within a Thrift file, and will be generated in each language minus JS ([see post](https://fb.workplace.com/groups/thriftusers/posts/887649301944483/?comment_id=887654188610661)). Every Base type can be made into a constant.

```
const i64 Foo = 11

const list<i32> Bar = [2, 3, 5, 7, Foo]

struct Baz {
  1: list<i32> field1 = Bar,
}
```
---

## **Typedefs**

Typedefs can be specified within the IDL, and will generate typedefs in each language.

```
typedef map<string, string> StringMap

struct Foo {
  1: i64 field1 = 101,
  2: StringMap field2,
  3: StringMap field3 = {'key' : 'value'}
}
```
---

## **Includes**

Thrift allows including other Thrift files to use any of its `const`, `enum`, `struct`, `union`, `exception`, or `services`. These can be referenced from another file by prepending the filename to the struct or service name. This means you cannot include multiple files with the same file name, only differing by path; confusion will reign. There is no aliasing.

```
include "common/example/if/Foo.thrift"

// Given a struct Bar in Foo.thrift

struct Baz {
  1: Foo.Bar field1,
}
```
---

## **Namespaces**

Namespaces can be specified by language for each enum, struct, exception, and service. Nested namespaces should use a dot `.` to separate each namespace.

```
namespace cpp2 foo.bar
namespace hack FooBar
namespace java com.company.foo.bar
namespace py bar.foo

struct Baz {
  1: i64 field1,
}
```
---

## **Package**

We can specify package name for each thrift file. e.g., `facebook.com/path/to`.

* Package provides default namespace for supported languages to prevent naming conflicts.
* We can apply file level annotation to the package keyword.

### Default Namespace

Package name contains "domain/paths". It implies following namespace

* C++: {reverse(domain)[1:]}.{paths}
* py3: {reverse(domain)[1:]}.{paths[:-2] if paths[-1] == filename else paths}
* hack: {paths}
* java: {reverse(domain)}.{paths}

NOTE: User can override default namespace explicitly by using `namespace` keyword for each languages.


Here is an example

```
package "domain.com/path/to/file"
```
This package generates following namespace

```
namespace cpp2 domain.path.to.file
namespace py3 domain.path.to
namespace hack path.to.file
namespace php path.to.file
namespace java2 com.domain.path.to.file
namespace java.swift com.domain.path.to.file
```
If package name doesn't contain filename, it generates different namespace for `py3`

```
package "domain.com/path/to"
```
This package generates following namespace

```
namespace cpp2 domain.path.to
namespace py3 domain.path.to
namespace hack path.to
namespace php path.to
namespace java2 com.domain.path.to
namespace java.swift com.domain.path.to
```
### File level annotation

We can apply annotation on the package keyword, in that case the annotation will be applied to the whole file. e.g.

```
@cpp.TerseWrite
package "foo.bar/baz"
```
This turns all fields in this file into terse-write fields when possible.

---

## **Services**

An interface for RPC is defined in a Thrift file as a service.

Each service has a set of methods. Each method has a unique name, takes a list of inputs, may return an output, and may define special application exceptions. Inputs are numbered like structs, but it is best to have a request object instead of listing a number of members.

Servers will either return the result of the appropriate type, or can throw exceptions to the client. If the exception type is a Thrift exception, then all of the members of the exception will be serialized with the exception. Otherwise, a TApplicationException will be thrown (the base exception type), and the members of the object that was thrown will not be serialized (except the message).

A `oneway` method will fire and forget. The client does not wait for a response, so there is no return type or application exceptions.

```
struct Foo {
  1: i64 field1 = 101,
  2: string field2,
}

exception BarException {
  1: string message,
  2: i64 errorCode,
}

struct GetFoosRequest {
  1: list<i64> ids,
}

service Bar {
  void ping() throws (1: BarException e),
  Foo getFoos(1: GetFoosRequest request) throws (1: BarException e),
  oneway void fireAndForget(),

  // Methods can also return streams. Streams allow for the flow of ordered
  // messages from the producer to the consumer. Not supported by all
  // languages.
  stream<T1> streamMethod(1: MyRequest request), //5
}
```
> **WARNING:** New inputs fields can be added to a method, but it is better to define an input struct and add members to it instead. The server cannot distinguish between missing arguments and default values, so a request object is better.


---

## **Comments**

Comments can be added in bash and c style.

```
/*
This is a comment
*/
struct Foo {
  1: i64 field1, // This is a comment
  // This is a comment
  2: required string field2,
  // 3: i32 deprecated, This whole line is a comment
  4: optional list<double> field4,
}
```
> Note: It is a good practice to comment out deprecated fields as a reminder to not to reuse it - see [Thrift Versioning](https://www.internalfb.com/intern/wiki/Thrift/Versioning).
