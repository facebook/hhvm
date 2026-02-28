---
title: BasePoll
---

:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Asynchronous equivalent of mechanisms such as epoll(), poll() and select()




Read the warnings here first, then see the ` Poll ` and `` KeyedPoll ``
instantiable subclasses.




Transforms a set of Awaitables to an asynchronous iterator that produces
results of these Awaitables as soon as they are ready. The order of results
is not guaranteed in any way. New Awaitables can be added to the Poll
while it is being iterated.




This mechanism has two primary use cases:




1) Speculatively issuing non-CPU-intensive requests to different backends
   with very high processing latency, waiting for the first satisfying
   result and ignoring all remaining requests.




   Example: cross-DC memcache requests

1) Processing relatively small number of high level results in the order
   of completion and flushing the output to the user.




   Example: pagelets, multiple GraphQL queries, streamable GraphQL queries





:::warning




This is a very heavy-weight mechanism with non-trivial CPU cost. NEVER use
this in the following situations:




1. Waiting for the first available result and ignoring the rest of work,
   unless the processing latency is extremely high (10ms or more) and
   the CPU cost of ignored work is negligible. Note: the ignored work
   will still be computed and will delay your processing anyway if it's
   CPU costly.

1. Reordering huge amount of intermediary results. This is currently known
   to be CPU-intensive.





:::




## Interface Synopsis




``` Hack
namespace HH\Lib\Async;

abstract class BasePoll {...}
```




### Public Methods




- [` ::create(): this `](/hsl/Classes/HH.Lib.Async/BasePoll/create/)
- [` ->hasNext(): bool `](/hsl/Classes/HH.Lib.Async/BasePoll/hasNext/)
- [` ->next(): Awaitable<?(Tk, Tv)> `](/hsl/Classes/HH.Lib.Async/BasePoll/next/)







### Protected Methods




+ [` ::fromImpl(KeyedTraversable<Tk, Awaitable<Tv>> $awaitables): this `](/hsl/Classes/HH.Lib.Async/BasePoll/fromImpl/)
+ [` ->addImpl(Tk $key, Awaitable<Tv> $awaitable): void `](/hsl/Classes/HH.Lib.Async/BasePoll/addImpl/)
+ [` ->addMultiImpl(KeyedTraversable<Tk, Awaitable<Tv>> $awaitables): void `](/hsl/Classes/HH.Lib.Async/BasePoll/addMultiImpl/)
<!-- HHAPIDOC -->
