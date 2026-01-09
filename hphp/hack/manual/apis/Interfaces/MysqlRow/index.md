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




+ [` ->at(mixed $field): ?mixed `](/docs/apis/Interfaces/MysqlRow/at/)
+ [` ->count(): int `](/docs/apis/Interfaces/MysqlRow/count/)
+ [` ->fieldType(mixed $field): int `](/docs/apis/Interfaces/MysqlRow/fieldType/)
+ [` ->getFieldAsDouble(mixed $field): float `](/docs/apis/Interfaces/MysqlRow/getFieldAsDouble/)
+ [` ->getFieldAsInt(mixed $field): int `](/docs/apis/Interfaces/MysqlRow/getFieldAsInt/)
+ [` ->getFieldAsString(mixed $field): string `](/docs/apis/Interfaces/MysqlRow/getFieldAsString/)
+ [` ->getIterator(): KeyedIterator<string, mixed> `](/docs/apis/Interfaces/MysqlRow/getIterator/)
+ [` ->isNull(mixed $field): bool `](/docs/apis/Interfaces/MysqlRow/isNull/)
<!-- HHAPIDOC -->
