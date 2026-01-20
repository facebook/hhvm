
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

:::warning
**Deprecated:** Use `C\firstx(await #A)`.
:::




Returns the first element of the result of the given Awaitable, or null if
the Traversable is empty




``` Hack
namespace HH\Lib\C;

function first_async<T>(
  Awaitable<Traversable<T>> $awaitable,
): Awaitable<?T>;
```




For non-Awaitable Traversables, see ` C\first `.




Time complexity: O(1)
Space complexity: O(1)




## Parameters




+ [` Awaitable<Traversable<T>> `](/apis/Classes/HH/Awaitable/)`` $awaitable ``




## Returns




* [` Awaitable<?T> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
