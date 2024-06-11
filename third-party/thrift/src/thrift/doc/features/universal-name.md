# Universal Names

A universal name uniquely identifies a Thrift [definition](/idl/index.md#definitions) at runtime, regardless of which Thrift file it is defined in. It looks like a partial URI (`"fbthrift://"` is implied) and has the following format:

```grammar
universal_name ::=  domain_path '/' identifier
```

where `domain_path` is defined in [Package Declaration](/idl/index.md#package-declaration).

A universal name is either [generated automatically](#package) based on the package name or [specified manually](#annotation) using the `@thrift.Uri` annotation.

You should ensure that universal names for your definitions are unique. For generated universal names, follow the recommendations on [naming packages](/idl/index.md#package-declaration). For `@thrift.Uri`, combine a domain name that belongs to you or your organization with a path, possibly consisting of multiple components, that uniquely identifies this definition within the domain/organization.

:::caution
Universal name conflicts can be difficult to resolve, so choose these names carefully and prefer longer, more specific names to shorter, generic ones.
:::

## Package

Universal names are generated for each Thrift [definition](/idl/index.md#definitions) by combining the package name with the definition name. In the example below, `SearchRequest` has the universal name `meta.com/search/SearchRequest`, and `SearchException` has the universal name `meta.com/search/SearchException`.

```thrift
package "meta.com/search"

struct SearchRequest {
  1: string query;
  2: i32 numResults;
}

exception SearchException {
  1: string errorMessage;
  2: i64 errorCode;
}
```

## Annotation

Universal names can also be defined or overridden with the `@thrift.Uri` annotation.

```thrift
@thrift.Uri{value = "meta.com/search/NewSearchRequest"}
struct SearchRequest {
  1: string query;
  2: i32 numResults;
}
```

```thrift
package "meta.com/search"

@thrift.Uri{value = "meta.com/search/NewSearchRequest"}
struct SearchRequest {
  1: string query;
  2: i32 numResults;
}
```

## Hashing

While a universal name is great for discovery and enforcing global uniqueness, its length is not fixed and can become large. Instead, a hash of the URI is usually preferred. Thrift Any and SemiAny are examples where a hash **may** be used. The hash is calculated as SHA2-256, prefixing the universal name with `"fbthrift://"`. The output is 32 bytes in length, but the prefix of the hash can be used to uniquely identify a universal name. The default length of the hash is 16 bytes, and the minimum is defined as 8 bytes.

The table below shows an example of the universal name, hash, and 8-byte prefixes.

| Universal Name | Prefix (8 bytes) | Full SHA2-256 Hash |
| -------------- | ---------------- | ------------------ |
| meta.com/conformance/foo | 09d3e751c86fde6e | 09d3e751c86fde6ecdf9cd195a1f60f8dd326b0b2c85b68830dfee4698ebe938 |
| meta.com/conformance/bar | 5b3919ca705489d9 | 5b3919ca705489d9291cb7dcf8ed504acda4c2f5e28ac4ea1213cfc208a550e2 |
| meta.com/conformance/baz | 0d80a6583b14f2e2 | 0d80a6583b14f2e2e3f38309621a4992ed7b93d3a7850d825a67c1b7f7206d27|

An 8-byte prefix allows 2<sup>64</sup> unique values before a collision occurs.
