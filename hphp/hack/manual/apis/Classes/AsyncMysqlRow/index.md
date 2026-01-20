---
title: AsyncMysqlRow
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

A class to represent a row




You can think of a row just like you do a database row that might be
returned as a result from a query. The row has values associated with
each column.




## Guides




+ [Introduction](</hack/asynchronous-operations/introduction>)
+ [Extensions](</hack/asynchronous-operations/extensions>)







## Interface Synopsis




``` Hack
final class AsyncMysqlRow implements MysqlRow {...}
```




### Public Methods




* [` ->at(mixed $field): mixed `](/apis/Classes/AsyncMysqlRow/at/)\
  Get field (column) value indexed by the `` field ``
* [` ->count(): int `](/apis/Classes/AsyncMysqlRow/count/)\
  Get the number of fields (columns) in the current row
* [` ->fieldType(mixed $field): int `](/apis/Classes/AsyncMysqlRow/fieldType/)\
  Returns the type of the field (column)
* [` ->getFieldAsDouble(mixed $field): float `](/apis/Classes/AsyncMysqlRow/getFieldAsDouble/)\
  Get a certain field (column) value as a `` double ``
* [` ->getFieldAsInt(mixed $field): int `](/apis/Classes/AsyncMysqlRow/getFieldAsInt/)\
  Get a certain field (column) value as an `` int ``
* [` ->getFieldAsString(mixed $field): string `](/apis/Classes/AsyncMysqlRow/getFieldAsString/)\
  Get a certain field (column) value as a `` string ``
* [` ->getIterator(): KeyedIterator<string, mixed> `](/apis/Classes/AsyncMysqlRow/getIterator/)\
  Get the iterator over the fields in the current row
* [` ->isNull(mixed $field): bool `](/apis/Classes/AsyncMysqlRow/isNull/)\
  Returns whether a field (column) value is `` null ``
<!-- HHAPIDOC -->
