
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Wait for a given Awaitable to finish and return its result




``` Hack
namespace HH\Asio;

function join<T>(
  Awaitable<T> $awaitable,
): T;
```




Launches a new instance of scheduler to drive asynchronous execution
until the provided Awaitable is finished.




## Parameters




+ [` Awaitable<T> `](/apis/Classes/HH/Awaitable/)`` $awaitable ``




## Returns




* ` T `
<!-- HHAPIDOC -->
