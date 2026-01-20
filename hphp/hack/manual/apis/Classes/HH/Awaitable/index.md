---
title: Awaitable
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

An ` Awaitable ` value represents a value that is fetched
asynchronously, such as a database access




` Awaitable ` values are
usually returned by `` async `` functions.




Use ` await ` to wait for a single `` Awaitable `` value. If you have
multiple ``` Awaitable ```s and you want to wait for all of them
together, use ```` concurrent ```` or helper functions like
````` Vec\map_async `````.




` Awaitable ` is not multithreading. Hack is single threaded, so
`` Awaitable `` allows you to wait for multiple external results at
once, rather than sequentially.




## Interface Synopsis




``` Hack
namespace HH;

abstract class Awaitable {...}
```




### Public Methods




+ [` ::setOnIOWaitEnterCallback(mixed $callback): void `](/apis/Classes/HH/Awaitable/setOnIOWaitEnterCallback/)\
  Set callback for when the scheduler enters I/O wait
+ [` ::setOnIOWaitExitCallback(mixed $callback): void `](/apis/Classes/HH/Awaitable/setOnIOWaitExitCallback/)\
  Set callback for when the scheduler exits I/O wait
+ [` ::setOnJoinCallback(mixed $callback): void `](/apis/Classes/HH/Awaitable/setOnJoinCallback/)\
  Set callback for when \\HH\\Asio\\join() is called
+ [` ->getName(): string `](/apis/Classes/HH/Awaitable/getName/)\
  Get name of the operation behind this wait handle
+ [` ->isFailed(): bool `](/apis/Classes/HH/Awaitable/isFailed/)\
  Check if this wait handle failed
+ [` ->isFinished(): bool `](/apis/Classes/HH/Awaitable/isFinished/)\
  Check if this wait handle finished (succeeded or failed)
+ [` ->isSucceeded(): bool `](/apis/Classes/HH/Awaitable/isSucceeded/)\
  Check if this wait handle succeeded
<!-- HHAPIDOC -->
