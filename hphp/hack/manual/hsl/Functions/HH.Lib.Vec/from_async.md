
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new vec with each value ` await `ed in parallel




``` Hack
namespace HH\Lib\Vec;

function from_async<Tv>(
  Traversable<Awaitable<Tv>> $awaitables,
): Awaitable<vec<Tv>>;
```




Time complexity: O(n * a), where a is the complexity of synchronous
portions of each Awaitable
Space complexity: O(n)




The IO operations for each Awaitable will happen in parallel.




## Parameters




+ [` Traversable<Awaitable<Tv>> `](/apis/Interfaces/HH/Traversable/)`` $awaitables ``




## Returns




* [` Awaitable<vec<Tv>> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
