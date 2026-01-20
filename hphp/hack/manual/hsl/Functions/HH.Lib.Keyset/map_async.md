
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new keyset where the value is the result of calling the
given async function on the original values in the given traversable




``` Hack
namespace HH\Lib\Keyset;

function map_async<Tv, Tk as arraykey>(
  Traversable<Tv> $traversable,
  (function(Tv): Awaitable<Tk>) $async_func,
): Awaitable<keyset<Tk>>;
```




Time complexity: O(n * f), where f is the complexity of the synchronous
portions of ` $async_func `
Space complexity: O(n)




The IO operations for each of calls to ` $async_func ` will happen in
parallel.




## Parameters




+ [` Traversable<Tv> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` (function(Tv): Awaitable<Tk>) $async_func `




## Returns




* [` Awaitable<keyset<Tk>> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
