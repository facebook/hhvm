---
title: AsyncMysqlRowIterator
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

A class to represent an iterator over the fields (columns) in a row




You can iterate over all the fields (columns) of an ` AsyncMysqlBlock ` one by
one until the iterator is not valid any longer.




## Guides




+ [Introduction](</hack/asynchronous-operations/introduction>)
+ [Extensions](</hack/asynchronous-operations/extensions>)







## Interface Synopsis




``` Hack
final class AsyncMysqlRowIterator implements HH\KeyedIterator {...}
```




### Public Methods




* [` ->current(): string `](/apis/Classes/AsyncMysqlRowIterator/current/)\
  Get the current field (column) name
* [` ->key(): int `](/apis/Classes/AsyncMysqlRowIterator/key/)\
  Get the current field (column) number
* [` ->next(): void `](/apis/Classes/AsyncMysqlRowIterator/next/)\
  Advance the iterator to the next field (column)
* [` ->rewind(): void `](/apis/Classes/AsyncMysqlRowIterator/rewind/)\
  Reset the iterator to the first field (column)
* [` ->valid(): bool `](/apis/Classes/AsyncMysqlRowIterator/valid/)\
  Check if the iterator is at a valid field (column)
<!-- HHAPIDOC -->
