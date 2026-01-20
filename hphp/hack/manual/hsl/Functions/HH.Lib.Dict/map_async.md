
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict where each value is the result of calling the given
async function on the original value




``` Hack
namespace HH\Lib\Dict;

function map_async<Tk as arraykey, Tv1, Tv2>(
  KeyedTraversable<Tk, Tv1> $traversable,
  (function(Tv1): Awaitable<Tv2>) $value_func,
): Awaitable<dict<Tk, Tv2>>;
```




For non-async functions, see ` Dict\map() `.




Time complexity: O(n * f), where f is the complexity of the synchronous
portions of ` $async_func `
Space complexity: O(n)




The IO operations for each of calls to ` $async_func ` will happen in
parallel.




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Tv1> $traversable ``
+ ` (function(Tv1): Awaitable<Tv2>) $value_func `




## Returns




* [` Awaitable<dict<Tk, `](/apis/Classes/HH/Awaitable/)`` Tv2>> ``
<!-- HHAPIDOC -->
