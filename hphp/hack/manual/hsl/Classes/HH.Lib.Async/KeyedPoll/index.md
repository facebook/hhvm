---
title: KeyedPoll
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

A keyed variant of ` Poll `




See ` Poll ` if you do not need to preserve keys.




Keys are retrieved with:




```
foreach ($keyed_poll await as $k => $v) {
```




:::warning




See detailed warning for ` BasePoll `




:::




## Interface Synopsis




``` Hack
namespace HH\Lib\Async;

final class KeyedPoll extends BasePoll<Tk, Tv> implements AsyncKeyedIterator<Tk, Tv> {...}
```




### Public Methods




+ [` ::from(KeyedTraversable<Tk, Awaitable<Tv>> $awaitables): this `](/docs/hsl/Classes/HH.Lib.Async/KeyedPoll/from/)\
  Create a `` KeyedPoll `` from the specified list of awaitables
+ [` ->add(Tk $key, Awaitable<Tv> $awaitable): void `](/docs/hsl/Classes/HH.Lib.Async/KeyedPoll/add/)\
  Add a single awaitable to the poll
+ [` ->addMulti(KeyedTraversable<Tk, Awaitable<Tv>> $awaitables): void `](/docs/hsl/Classes/HH.Lib.Async/KeyedPoll/addMulti/)\
  Add multiple keys and awaitables to the poll







### Public Methods ([` HH\Lib\Async\BasePoll `](/docs/hsl/Classes/HH.Lib.Async/BasePoll/))




* [` ::create(): this `](/docs/hsl/Classes/HH.Lib.Async/BasePoll/create/)
* [` ->hasNext(): bool `](/docs/hsl/Classes/HH.Lib.Async/BasePoll/hasNext/)
* [` ->next(): Awaitable<?(Tk, Tv)> `](/docs/hsl/Classes/HH.Lib.Async/BasePoll/next/)







### Protected Methods ([` HH\Lib\Async\BasePoll `](/docs/hsl/Classes/HH.Lib.Async/BasePoll/))




- [` ::fromImpl(KeyedTraversable<Tk, Awaitable<Tv>> $awaitables): this `](/docs/hsl/Classes/HH.Lib.Async/BasePoll/fromImpl/)
- [` ->addImpl(Tk $key, Awaitable<Tv> $awaitable): void `](/docs/hsl/Classes/HH.Lib.Async/BasePoll/addImpl/)
- [` ->addMultiImpl(KeyedTraversable<Tk, Awaitable<Tv>> $awaitables): void `](/docs/hsl/Classes/HH.Lib.Async/BasePoll/addMultiImpl/)
<!-- HHAPIDOC -->
