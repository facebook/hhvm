---
title: Semaphore
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Run an operation with a limit on number of ongoing asynchronous jobs




All operations must have the same input type (` Tin `) and output type (`` Tout ``),
and be processed by the same function; ``` Tin ``` may be a callable invoked by the
function for maximum flexibility, however this pattern is best avoided in favor
of creating semaphores with a more narrow process.




Use ` genWaitFor() ` to retrieve a `` Tout `` from a ``` Tin ```.




## Interface Synopsis




``` Hack
namespace HH\Lib\Async;

final class Semaphore {...}
```




### Public Methods




+ [` ->__construct(int $concurrentLimit, (function(Tin): Awaitable<Tout>) $f) `](/hsl/Classes/HH.Lib.Async/Semaphore/__construct/)\
  Create a semaphore
+ [` ->waitForAsync(Tin $value): Awaitable<Tout> `](/hsl/Classes/HH.Lib.Async/Semaphore/waitForAsync/)\
  Produce a `` Tout `` from a ``` Tin ```, respecting the concurrency limit
<!-- HHAPIDOC -->
