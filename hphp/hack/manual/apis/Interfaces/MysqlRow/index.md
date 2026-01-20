---
title: MysqlRow
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

## Interface Synopsis




``` Hack
interface MysqlRow implements Countable, IteratorAggregate<mixed>, HH\KeyedTraversable<string, mixed> {...}
```




### Public Methods




+ [` ->at(mixed $field): ?mixed `](/apis/Interfaces/MysqlRow/at/)
+ [` ->count(): int `](/apis/Interfaces/MysqlRow/count/)
+ [` ->fieldType(mixed $field): int `](/apis/Interfaces/MysqlRow/fieldType/)
+ [` ->getFieldAsDouble(mixed $field): float `](/apis/Interfaces/MysqlRow/getFieldAsDouble/)
+ [` ->getFieldAsInt(mixed $field): int `](/apis/Interfaces/MysqlRow/getFieldAsInt/)
+ [` ->getFieldAsString(mixed $field): string `](/apis/Interfaces/MysqlRow/getFieldAsString/)
+ [` ->getIterator(): KeyedIterator<string, mixed> `](/apis/Interfaces/MysqlRow/getIterator/)
+ [` ->isNull(mixed $field): bool `](/apis/Interfaces/MysqlRow/isNull/)
<!-- HHAPIDOC -->
