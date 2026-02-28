
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Asynchronously run a one-shot Watchman query




``` Hack
namespace HH;

function watchman_query(
  mixed $json_query,
  ?string $socket_name = NULL,
): Awaitable<WatchmanResult>;
```




See the Watchman docs for
details of queries and responses.




## Parameters




+ ` mixed $json_query `
+ ` ?string $socket_name = NULL `




## Returns




* [` Awaitable<WatchmanResult> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
