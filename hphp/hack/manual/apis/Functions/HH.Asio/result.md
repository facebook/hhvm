
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get result of an already finished Awaitable




``` Hack
namespace HH\Asio;

function result<T>(
  Awaitable<T> $awaitable,
): T;
```




Throws an InvalidOperationException if the Awaitable is not finished.




## Parameters




+ [` Awaitable<T> `](/apis/Classes/HH/Awaitable/)`` $awaitable ``




## Returns




* ` T `
<!-- HHAPIDOC -->
