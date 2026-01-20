
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Connect to a socket asynchronously, returning a non-disposable handle




``` Hack
namespace HH\Lib\TCP;

function connect_async(
  string $host,
  int $port,
  ConnectOptions $opts = dict [
],
): Awaitable<CloseableSocket>;
```




If using IPv6 with a fallback to IPv4 with a connection timeout, the timeout
will apply separately to the IPv4 and IPv6 connection attempts.




## Parameters




+ ` string $host `
+ ` int $port `
+ ` ConnectOptions $opts = dict [ ] `




## Returns




* [` Awaitable<CloseableSocket> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
