---
title: AsyncMysqlRowBlock
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Represents a row block




A row block is either a full or partial set of result rows from a MySQL
query.




In a query result, the sum total of all the row blocks is the full result
of the query. Most of the time there is only one row block per query result
since the query was never interrupted or otherwise deterred by some outside
condition like exceeding network packet parameters.




You can get an instance of ` AsyncMysqlRowBlock ` via the
[` AsyncMysqlQueryResult::rowBlocks() `](/docs/apis/Classes/AsyncMysqlQueryResult/rowBlocks/) call.




## Guides




+ [Introduction](</docs/hack/asynchronous-operations/introduction>)
+ [Extensions](</docs/hack/asynchronous-operations/extensions>)







## Interface Synopsis




``` Hack
final class AsyncMysqlRowBlock implements IteratorAggregate, Countable, KeyedTraversable<int, AsyncMysqlRow> {...}
```




### Public Methods




* [` ->at(int $row, mixed $field): mixed `](/docs/apis/Classes/AsyncMysqlRowBlock/at/)\
  Get a field (column) value
* [` ->count(): int `](/docs/apis/Classes/AsyncMysqlRowBlock/count/)\
  Returns the number of rows in the current row block
* [` ->fieldFlags(mixed $field): int `](/docs/apis/Classes/AsyncMysqlRowBlock/fieldFlags/)\
  Returns the flags of the field (column)
* [` ->fieldName(int $field): string `](/docs/apis/Classes/AsyncMysqlRowBlock/fieldName/)\
  Returns the name of the field (column)
* [` ->fieldType(mixed $field): int `](/docs/apis/Classes/AsyncMysqlRowBlock/fieldType/)\
  Returns the type of the field (column)
* [` ->fieldsCount(): int `](/docs/apis/Classes/AsyncMysqlRowBlock/fieldsCount/)\
  Returns the number of fields (columns) associated with the current row
  block
* [` ->getFieldAsDouble(int $row, mixed $field): float `](/docs/apis/Classes/AsyncMysqlRowBlock/getFieldAsDouble/)\
  Get a certain field (column) value from a certain row as `` double ``
* [` ->getFieldAsInt(int $row, mixed $field): int `](/docs/apis/Classes/AsyncMysqlRowBlock/getFieldAsInt/)\
  Get a certain field (column) value from a certain row as `` int ``
* [` ->getFieldAsString(int $row, mixed $field): string `](/docs/apis/Classes/AsyncMysqlRowBlock/getFieldAsString/)\
  Get a certain field (column) value from a certain row as `` string ``
* [` ->getIterator(): KeyedIterator<int, AsyncMysqlRow> `](/docs/apis/Classes/AsyncMysqlRowBlock/getIterator/)\
  Get the iterator for the rows in the block
* [` ->getRow(int $row): AsyncMysqlRow `](/docs/apis/Classes/AsyncMysqlRowBlock/getRow/)\
  Get a certain row in the current row block
* [` ->isEmpty(): bool `](/docs/apis/Classes/AsyncMysqlRowBlock/isEmpty/)\
  Returns whether there were any rows are returned in the current row block
* [` ->isNull(int $row, mixed $field): bool `](/docs/apis/Classes/AsyncMysqlRowBlock/isNull/)\
  Returns whether a field (column) value is `` null ``
<!-- HHAPIDOC -->
