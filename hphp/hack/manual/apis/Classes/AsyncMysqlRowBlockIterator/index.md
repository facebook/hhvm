---
title: AsyncMysqlRowBlockIterator
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

A class to represent an iterator over the rows of a [` AsyncMysqlRowBlock `](/apis/Classes/AsyncMysqlRowBlock/)




You can iterate over all the rows of an [` AsyncMysqlRowBlock `](/apis/Classes/AsyncMysqlRowBlock/) one by one until
the iterator is not valid any longer.




## Guides




+ [Introduction](</hack/asynchronous-operations/introduction>)
+ [Extensions](</hack/asynchronous-operations/extensions>)







## Interface Synopsis




``` Hack
final class AsyncMysqlRowBlockIterator implements HH\KeyedIterator {...}
```




### Public Methods




* [` ->current(): AsyncMysqlRow `](/apis/Classes/AsyncMysqlRowBlockIterator/current/)\
  Get the current row

* [` ->key(): int `](/apis/Classes/AsyncMysqlRowBlockIterator/key/)\
  Get the current row number

* [` ->next(): void `](/apis/Classes/AsyncMysqlRowBlockIterator/next/)\
  Advance the iterator to the next row

* [` ->rewind(): void `](/apis/Classes/AsyncMysqlRowBlockIterator/rewind/)\
  Reset the iterator to the first row

* [` ->valid(): bool `](/apis/Classes/AsyncMysqlRowBlockIterator/valid/)\
  Check if iterator is at a valid [` AsyncMysqlRow `](/apis/Classes/AsyncMysqlRow/)

<!-- HHAPIDOC -->
