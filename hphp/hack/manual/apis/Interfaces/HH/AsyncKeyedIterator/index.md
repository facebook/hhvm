---
title: AsyncKeyedIterator
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Allows for the iteration over the keys and values provided by an ` async `
function




If an ` async ` function returns an [` AsyncIterator<Tk, `](/apis/Interfaces/HH/AsyncIterator/)`` Tv> ``, then you can
iterate over the ``` Tk ``` and ```` Tv ```` values returned from that function.




```
async function countdown(int $start): AsyncIterator<int, string> { ... }

async function use_countdown(): Awaitable<void> {
  $async_iter = countdown(100);
  foreach ($async_gen await as $num => $str) { ... }
}
```




## Guides




+ [Introduction](</hack/asynchronous-operations/introduction>)
+ [Guidelines](</hack/asynchronous-operations/guidelines>)







## Interface Synopsis




``` Hack
namespace HH;

interface AsyncKeyedIterator implements AsyncIterator<Tv> {...}
```




### Public Methods




* [` ->next(): Awaitable<?(Tk, Tv)> `](/apis/Interfaces/HH/AsyncKeyedIterator/next/)\
  Move the async iterator to the next [` Awaitable `](/apis/Classes/HH/Awaitable/) position
<!-- HHAPIDOC -->
