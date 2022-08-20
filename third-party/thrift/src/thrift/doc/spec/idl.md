---
state: experimental
---

# Interface Definition Language (IDL)

How to write a Thrift program.

## Style

The following naming style **should** be used in all Thrift IDL files for consistency and clarity.

### Naming

Use snake-case for file names, as they function like 'module' names when included in other thrift files, for example: `standard.thrift`, `security_domain.thrift.`

Use upper-camel-case (a.k.a. PascalCase) for [definitions](definition/index.md).

- `struct MyStruct { ... }`, not `myStruct`
- `typedef int MyInt`, not `my_int`

Use lower-camel-case for identifiers, including consts.

- `1: string tierName`, not `tier_name`
- `const i64 minId = 1`, not `kMinID`

Don’t capitalize acronyms; they follow the same camel-case rules.

- `serviceId`, not `serviceID`
- `service HttpServer { ... }`, not `HTTPServer`

Use types to endow values with semantics.

- [`type.Duration`](https://github.com/facebook/fbthrift/tree/main/thrift/lib/thrift/type.thrift#Duration) `timeout`, not `i64 timeout`
- [`type.Timestamp`](https://github.com/facebook/fbthrift/tree/main/thrift/lib/thrift/type.thrift#Timestamp)  `createTime`, not `i64 timestamp`.

When types are impractical for dimensional values, include units, for example `ttlMs` instead of `ttl`.

## Formatting

How to have pretty IDL files.

## Linting

How to have tidy IDL files.

## Grammar

The full IDL grammar.
