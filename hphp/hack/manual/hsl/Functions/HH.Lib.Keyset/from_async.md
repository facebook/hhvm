
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new keyset containing the awaited result of the given Awaitables




``` Hack
namespace HH\Lib\Keyset;

function from_async<Tv as arraykey>(
  Traversable<Awaitable<Tv>> $awaitables,
): Awaitable<keyset<Tv>>;
```




Time complexity: O(n * a), where a is the complexity of the synchronous
portions of each Awaitable
Space complexity: O(n)




The IO operations for each Awaitable will happen in parallel.




## Parameters




+ [` Traversable<Awaitable<Tv>> `](/apis/Interfaces/HH/Traversable/)`` $awaitables ``




## Returns




* [` Awaitable<keyset<Tv>> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
