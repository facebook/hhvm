
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a new dict where each value is the result of calling the given
async function on the corresponding key




``` Hack
namespace HH\Lib\Dict;

function from_keys_async<Tk as arraykey, Tv>(
  Traversable<Tk> $keys,
  (function(Tk): Awaitable<Tv>) $async_func,
): Awaitable<dict<Tk, Tv>>;
```




For non-async functions, see ` Dict\from_keys() `.




Time complexity: O(n * f), where f is the complexity of ` $async_func `
Space complexity: O(n)




## Parameters




+ [` Traversable<Tk> `](/apis/Interfaces/HH/Traversable/)`` $keys ``
+ ` (function(Tk): Awaitable<Tv>) $async_func `




## Returns




* [` Awaitable<dict<Tk, `](/apis/Classes/HH/Awaitable/)`` Tv>> ``
<!-- HHAPIDOC -->
