
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict where each value is the result of calling the given
async function on the original key and value




``` Hack
namespace HH\Lib\Dict;

function map_with_key_async<Tk as arraykey, Tv1, Tv2>(
  KeyedTraversable<Tk, Tv1> $traversable,
  (function(Tk, Tv1): Awaitable<Tv2>) $async_func,
): Awaitable<dict<Tk, Tv2>>;
```




For non-async functions, see ` Dict\map() `.




Time complexity: O(n * a), where a is the complexity of each Awaitable
Space complexity: O(n)




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv1> $traversable ``
+ ` (function(Tk, Tv1): Awaitable<Tv2>) $async_func `




## Returns




* [` Awaitable<dict<Tk, `](/apis/Classes/HH/Awaitable/)`` Tv2>> ``
<!-- HHAPIDOC -->
