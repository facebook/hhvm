---
title: IntervalTimer
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

A timer that periodically interrupts a request thread




## Interface Synopsis




``` Hack
class IntervalTimer {...}
```




### Public Methods




+ [` ->__construct(double $interval, float $initial, mixed $callback) `](/apis/Classes/IntervalTimer/__construct/)\
  Create a new interval timer
+ [` ->start(): void `](/apis/Classes/IntervalTimer/start/)\
  Start the timer
+ [` ->stop(): void `](/apis/Classes/IntervalTimer/stop/)\
  Stop the timer
<!-- HHAPIDOC -->
