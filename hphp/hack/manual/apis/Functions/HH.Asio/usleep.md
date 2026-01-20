
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Wait for a certain length of time before an async function does any more
work




``` Hack
namespace HH\Asio;

function usleep(
  int $usecs,
): Awaitable<void>;
```




This is similar to calling the PHP builtin
[` usleep `](<http://php.net/manual/en/function.usleep.php>) funciton, but is
in the context of async, meaning that other [` Awaitable `](/apis/Classes/HH/Awaitable/)s in the async
scheduler can run while the async function that called `` usleep() `` waits until
the length of time before asking to resume again.




## Parameters




+ ` int $usecs ` - The amount of time to wait, in microseconds.




## Returns




* [` Awaitable<void> `](/apis/Classes/HH/Awaitable/) - [` Awaitable `](/apis/Classes/HH/Awaitable/) of `` void ``.
<!-- HHAPIDOC -->
