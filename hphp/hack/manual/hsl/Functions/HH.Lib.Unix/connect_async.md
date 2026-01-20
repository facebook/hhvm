
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Asynchronously connect to the specified unix socket




``` Hack
namespace HH\Lib\Unix;

function connect_async(
  string $path,
  ConnectOptions $opts = dict [
],
): Awaitable<CloseableSocket>;
```




## Parameters




+ ` string $path `
+ ` ConnectOptions $opts = dict [ ] `




## Returns




* [` Awaitable<CloseableSocket> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
