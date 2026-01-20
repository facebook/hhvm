
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Cancel Awaitable, if it's still pending




``` Hack
namespace HH\Asio;

function cancel<T>(
  Awaitable<T, mixed> $awaitable,
  \Exception $exception,
): bool;
```




If Awaitable has not been completed yet, fails Awaitable with
$exception and returns true.
Otherwise, returns false.




Throws InvalidArgumentException, if Awaitable does not support cancellation.




## Parameters




+ [` Awaitable<T, `](/apis/Classes/HH/Awaitable/)`` mixed> $awaitable ``
+ ` \Exception $exception `




## Returns




* ` bool `
<!-- HHAPIDOC -->
