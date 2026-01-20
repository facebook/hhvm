
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

``` Hack
namespace HH;

function watchman_sync_sub(
  string $sub_name,
  int $timeout_ms = 0,
): Awaitable<bool>;
```




## Parameters




+ ` string $sub_name `
+ ` int $timeout_ms = 0 `




## Returns




* [` Awaitable<bool> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
