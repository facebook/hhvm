---
state: experimental
---

# Programs

A program consists of:

1. optional [includes](#includes),
1. the [package](#package) for the program,
1. optional [namespace](#namespaces) overrides, and
1. [definitions](#definitions).

## Universal Names

A Universal Name uniquely identifies a Thrift [definition](index.md), regardless of which program it is defined in. It **must** be a valid [URI](https://tools.ietf.org/html/rfc3986) that meets the following criteria:

- **Must not** have a scheme; "fbthrift://" is implied.
- **Must** include a domain name as the 'authority', with at least 2 domain labels.
- **Must** include a path (with at least 2 path segments for a definition).
- **Must not** have a query or a fragment.
- **Must** be in the following canonical/normalized form:
  - Domain name labels **must** be lowercase ASCII letters, numbers, -, or _.
  - Path segments **must** be ASCII letters, numbers, -, or _.

## Includes

Thrift allows including other Thrift files to use any of its consts and types. These can be referenced from another file by prepending the filename to the definition's name (i.e., `Filename.Identifier`).

```
IncludeDeclaration:
    include "{Path}/{Filename}.thrift"
```

- [Path](https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap03.html#tag_03_271) and [Filename](https://pubs.opengroup.org/onlinepubs/9699919799/basedefs/V1_chap03.html#tag_03_170) **must** be valid POSIX portable pathname and filename.

*Note* a program cannot include multiple files with the same file name, only differing by path.

## Package

A package declaration defines the default [Universal Name](#universal-names) and optional annotations, under which all [definitions](#definitions) in the program are declared:

```
PackageDeclaration:
    {Annotations}
    package "Domain/Identifier{/Identifier}";
```

For example:

```
include "thrift/annotations/thrift.thrift"

@thrift.Testing // Indicates all definitions in the program are for tests only.
package "test.dev/testing"

 // Has the universal name "test.dev/testing/MyInt" and inherits `@thrift.Testing`.
typedef int MyInt
```

## Namespaces

Language-specific overrides for the native module/package/namespace/etc abstraction, represented as a list of identifiers (names).

```
NamespaceDeclaration:
    namespace Language Identifier{.Identifier};
```

All Thrift implementations **must** define a default mapping from package to 'namespace'. For example, the following mapping are used for officially-supported Thrift languages:

- cpp2: `{reverse(domain)[1:]}.{path}`
- python: `{reverse(domain)[1:]}.{path[:-2] if path[-1] == filename else path}`
- hack: `{path}`
- java: `{reverse(domain)}.{path}`

For example,

```
package "domain.com/path/to/file"
```

This package name implies the following namespaces:

```
namespace cpp2 domain.path.to.file
namespace python domain.path.to
namespace py3 domain.path.to
namespace hack path.to.file
namespace php path.to.file
namespace java.swift com.domain.path.to.file
```

## Definitions

[Definitions](../index.md) **may** have an unstructured annotation `thrift.uri` as an [Universal Name](#universal-names). It **must** be globally unique. If a [Universal Name](#universal-names) is not specified, one will be generated using a package `{package}/{identifier}`. For example,

```
package "example.com/path/to/file"

struct Foo {
  1: i32 field;
}
```

This is equivalent to

```
package "example.com/path/to/file"

struct Foo {
  1: i32 field;
} (thrift.uri = "example.com/path/to/file/Foo")
```

## Abstract Syntax Tree (AST)

The parsed Thrift code is represented as an [Abstract Syntax Tree](https://github.com/facebook/fbthrift/tree/main/thrift/compiler/ast) that starts from `t_program` for each thrift file.
