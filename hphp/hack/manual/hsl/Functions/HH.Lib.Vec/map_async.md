
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec where each value is the result of calling the given
async function on the original value




``` Hack
namespace HH\Lib\Vec;

function map_async<Tv1, Tv2>(
  Traversable<Tv1> $traversable,
  (function(Tv1): Awaitable<Tv2>) $async_func,
): Awaitable<vec<Tv2>>;
```




For non-async functions, see ` Vec\map() `.




Time complexity: O(n * f), where ` f ` is the complexity of the synchronous
portions of `` $async_func ``
Space complexity: O(n)




The IO operations for each of calls to ` $async_func ` will happen in
parallel.




## Parameters




+ [` Traversable<Tv1> `](/apis/Interfaces/HH/Traversable/)`` $traversable ``
+ ` (function(Tv1): Awaitable<Tv2>) $async_func `




## Returns




* [` Awaitable<vec<Tv2>> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
