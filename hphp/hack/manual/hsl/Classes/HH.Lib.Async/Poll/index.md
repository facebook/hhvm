---
title: Poll
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

An async poll/select equivalent for traversables without a related key




:::warning




See detailed warning at top of ` BasePoll `




:::




## Interface Synopsis




``` Hack
namespace HH\Lib\Async;

final class Poll extends BasePoll<mixed, Tv> implements AsyncIterator<Tv> {...}
```




### Public Methods




+ [` ::from(Traversable<Awaitable<Tv>> $awaitables): this `](/hsl/Classes/HH.Lib.Async/Poll/from/)\
  Create a Poll from the specified list of awaitables
+ [` ->add(Awaitable<Tv> $awaitable): void `](/hsl/Classes/HH.Lib.Async/Poll/add/)\
  Add an additional awaitable to the poll
+ [` ->addMulti(Traversable<Awaitable<Tv>> $awaitables): void `](/hsl/Classes/HH.Lib.Async/Poll/addMulti/)\
  Add multiple additional awaitables to the poll
+ [` ->waitUntilEmptyAsync(): Awaitable<void> `](/hsl/Classes/HH.Lib.Async/Poll/waitUntilEmptyAsync/)\
  Wait for all polled [` Awaitable `](/apis/Classes/HH/Awaitable/)s, ignoring the results







### Public Methods ([` HH\Lib\Async\BasePoll `](/hsl/Classes/HH.Lib.Async/BasePoll/))




* [` ::create(): this `](/hsl/Classes/HH.Lib.Async/BasePoll/create/)
* [` ->hasNext(): bool `](/hsl/Classes/HH.Lib.Async/BasePoll/hasNext/)
* [` ->next(): Awaitable<?(Tk, Tv)> `](/hsl/Classes/HH.Lib.Async/BasePoll/next/)







### Protected Methods ([` HH\Lib\Async\BasePoll `](/hsl/Classes/HH.Lib.Async/BasePoll/))




- [` ::fromImpl(KeyedTraversable<Tk, Awaitable<Tv>> $awaitables): this `](/hsl/Classes/HH.Lib.Async/BasePoll/fromImpl/)
- [` ->addImpl(Tk $key, Awaitable<Tv> $awaitable): void `](/hsl/Classes/HH.Lib.Async/BasePoll/addImpl/)
- [` ->addMultiImpl(KeyedTraversable<Tk, Awaitable<Tv>> $awaitables): void `](/hsl/Classes/HH.Lib.Async/BasePoll/addMultiImpl/)
<!-- HHAPIDOC -->
