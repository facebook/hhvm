
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get the name of the Awaitable




``` Hack
namespace HH\Asio;

function name<T>(
  Awaitable<T, mixed> $awaitable,
): string;
```




## Parameters




+ [` Awaitable<T, `](/apis/Classes/HH/Awaitable/)`` mixed> $awaitable ``




## Returns




* ` string `
<!-- HHAPIDOC -->
