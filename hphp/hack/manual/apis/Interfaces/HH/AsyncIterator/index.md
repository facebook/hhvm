---
title: AsyncIterator
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Allows for the iteration over the values provided by an ` async ` function




If an ` async ` function returns an [` AsyncIterator<T> `](/apis/Interfaces/HH/AsyncIterator/), then you can iterate
over the `` T `` values returned from that function.




```
async function countdown(int $start): AsyncIterator<int> { ... }

async function use_countdown(): Awaitable<void> {
  $async_iter = countdown(100);
  foreach ($async_iter await as $value) { ... }
}
```




## Guides




+ [Introduction](</hack/asynchronous-operations/introduction>)
+ [Guidelines](</hack/asynchronous-operations/guidelines>)







## Interface Synopsis




``` Hack
namespace HH;

interface AsyncIterator {...}
```




### Public Methods




* [` ->next(): Awaitable<?(mixed, Tv)> `](/apis/Interfaces/HH/AsyncIterator/next/)\
  Move the async iterator to the next [` Awaitable `](/apis/Classes/HH/Awaitable/) position
<!-- HHAPIDOC -->
