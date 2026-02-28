# Utility Functions

Async can be used effectively with the basic, built in infrastructure in HHVM, along with some HSL functions. This basic infrastructure includes:
* [`async`](/hack/asynchronous-operations/introduction), [`await`](/hack/asynchronous-operations/awaitables), [`Awaitable`](/hack/asynchronous-operations/awaitables)
* `HH\Lib\Vec\from_async()`, `HH\Lib\Dict\from_async()`

However, there are cases when we want to convert some collection of values to awaitables or we want to filter some awaitables out
of a collection of awaitables. These types of scenarios come up when we are creating multiple awaitables to await in parallel. Some
of the functions that help with this are:

Name | Returns | Description
-----|---------|------------
`HH\Lib\Vec\filter_async<Tv>` | `Awaitable<vec<Tv>>` | Returns a new vec containing only the values for which the given async predicate returns `true`.
`HH\Lib\Vec\from_async<Tv>` | `Awaitable<vec<Tv>>` | Returns a new vec with each value awaited in parallel.
`HH\Lib\Vec\map_async<Tv1, Tv2>` | `Awaitable<vec<Tv2>>` | Returns a new vec where each value is the result of calling the given async function on the original value.
`HH\Lib\Dict\filter_async<Tk as arraykey, Tv>` | `Awaitable<dict<Tk,Tv>>` | Returns a new dict containing only the values for which the given async predicate returns `true`.
`HH\Lib\Dict\filter_with_key_async<Tk as arraykey, Tv>` | `Awaitable<dict<Tk,Tv>>` | Like `filter_async`, but lets you utilize the keys of your dict too.
`HH\Lib\Dict\from_async<Tk, Tv1>` | `Awaitable<dict<Tk,Tv2>>` | Returns a new dict where each value is the result of calling the given async function on the original value.
`HH\Lib\Dict\from_keys_async<Tk as arraykey, Tv>` | `Awaitable<dict<Tk,Tv>>` | Returns a new dict where each value is the result of calling the given async function on the corresponding key.
`HH\Lib\Dict\map_async<Tk as arraykey, Tv1, Tv2>` | `Awaitable<dict<Tk,Tv2>>` | Returns a new dict where each value is the result of calling the given async function on the original value.
`HH\Lib\Keyset\filter_async<Tv as arraykey>` | `Awaitable<keyset<Tv>>` | Returns a new keyset containing only the values for which the given async predicate returns `true`.
`HH\Lib\Keyset\from_async<Tv as arraykey>` | `Awaitable<keyset<Tv>>` | Returns a new keyset containing the awaited result of the given Awaitables.
`HH\Lib\Keyset\map_async<Tv, Tk as arraykey>` | `Awaitable<keyset<Tk>>` | Returns a new keyset where the value is the result of calling the given async function on the original values in the given traversable.

## Other Convenience Functions

There are three convenience-functions tailored for use with async. They are:

Name | Returns | Description
-----|---------|------------
`HH\Asio\usleep(int)` | `Awaitable<void>` | Wait a given length of time before an async function does more work.
`HH\Asio\later()` | `Awaitable<void>` | Reschedule the work of an async function until some undetermined point in the future.
`HH\Asio\wrap(Awaitable<Tv>)` | `Awaitable<ResultOrExceptionWrapper<Tv>>` | Wrap an `Awaitable` into an `Awaitable` of `ResultOrExceptionWrapper`.
