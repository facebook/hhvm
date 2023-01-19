---
state: experimental
---

# Universal Names

A Universal Name uniquely identifies a Thrift [definition](../idl/idl-reference#type-definitions), regardless of which program it is defined in. It **must** be a valid [URI](https://tools.ietf.org/html/rfc3986) that meets the following criteria:

- **Must not** have a scheme; "fbthrift://" is implied.
- **Must** include a domain name as the 'authority', with at least 2 domain labels.
- **Must** include a path (with at least 2 path segments for a definition).
- **Must not** have a query or a fragment.
- **Must** be in the following canonical/normalized form:
  - Domain name labels **must** be lowercase ASCII letters, numbers, -, or _.
  - Path segments **must** be ASCII letters, numbers, -, or _.

## Naming
Universal names are generated for each thrift [definition](../idl/idl-reference#type-definitions) using a package `{package}/{identifier}`. It can also be defined or overridden with an unstructured annotation `thrift.uri`. For example,

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

## Hashing
While a universal name (URI) is great for discovery and enforcing global uniqueness, its length is not fixed and can become large. Instead of URI, a hash of the URI is usually preferred. Conformance Any, Thrift Any and SemiAny are examples where hash **may** be used. The hash is calculated as SHA2-256 prefixing the URI with `fbthrift://` scheme. The output is 32 byte length data, but the prefix of the hash can be used to uniquely identify a universal name. Default length of the hash is 16 bytes and minimum is defined as 8 bytes.

The table below shows an example of the universal name, hash and 8 bytes prefix.

| Universal Name | SHA2-256 Hash | Prefix (8 bytes) |
| ----------- | ----------- | --------- |
| meta.com/conformance/foo | 09d3e751c86fde6ecdf9cd195a1f60f8dd326b0b2c85b68830dfee4698ebe938 | 09d3e751c86fde6e|
| meta.com/conformance/bar | 5b3919ca705489d9291cb7dcf8ed504acda4c2f5e28ac4ea1213cfc208a550e2 | 5b3919ca705489d9|
| meta.com/conformance/baz | 0d80a6583b14f2e2e3f38309621a4992ed7b93d3a7850d825a67c1b7f7206d27 | 0d80a6583b14f2e2|

8 byte prefix allows 2^64 unique values before a collision occurs.
