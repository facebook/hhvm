---
state: experimental
---

# Universal Names

Universal name for any thrift [definition](../idl/#type-definitions) can be defined either with `package` declaration or using `thrift.uri` annotation.

## Package
Universal names are generated for each thrift [definition](../idl/#type-definitions) using a package `{package}/{identifier}`. This is the preferred approach. In the example below, `Foo` has a universal name `meta.com/test/Foo` with a 8 byte hash prefix `ed794a6db87e6a6`. `MyException` has a universal name `meta.com/test/MyException` with a 8 byte hash prefix `2ec586517e69e2c8`. Refer thrift [universal name spec](../spec/definition/universal-name) for URI syntax and hash calculation.

```
package "meta.com/test"

struct Foo {
  1: i32 field;
}

exception MyException {
  1: string message;
}

```

## Annotation
Universal names can also be defined or overridden with an unstructured annotation `thrift.uri`.

```
struct Foo {
  1: i32 field;
} (thrift.uri = "meta.com/test/MyStruct")
```

```
package "meta.com/test"

struct Foo {
  1: i32 field;
} (thrift.uri = "meta.com/test/MyStruct")
```
