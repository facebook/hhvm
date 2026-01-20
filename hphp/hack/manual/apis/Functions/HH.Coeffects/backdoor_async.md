
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Creates an unsafe way to call a function by providing defaults coeffects




``` Hack
namespace HH\Coeffects;

function backdoor_async<Tout>(
  (function(): Awaitable<Tout>) $fn,
): Awaitable<Tout>;
```




EXTREMELY UNSAFE. USE WITH CAUTION.




## Parameters




+ ` (function(): Awaitable<Tout>) $fn `




## Returns




* [` Awaitable<Tout> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
