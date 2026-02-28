
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict with each value ` await `ed in parallel




``` Hack
namespace HH\Lib\Dict;

function from_async<Tk as arraykey, Tv>(
  KeyedTraversable<Tk, Awaitable<Tv>> $awaitables,
): Awaitable<dict<Tk, Tv>>;
```




Time complexity: O(n * a), where a is the complexity of the synchronous
portions of each Awaitable
Space complexity: O(n)




The IO operations for each Awaitable will happen in parallel.




## Parameters




+ [` KeyedTraversable<Tk, `](/apis/Interfaces/HH/KeyedTraversable/)`` Awaitable<Tv>> $awaitables ``




## Returns




* [` Awaitable<dict<Tk, `](/apis/Classes/HH/Awaitable/)`` Tv>> ``
<!-- HHAPIDOC -->
