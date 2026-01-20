
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Reschedule the work of an async function until some other time in the
future




``` Hack
namespace HH\Asio;

function later(): Awaitable<void>;
```




The common use case for this is if your async function actually has to wait
for some blocking call, you can tell other [` Awaitable `](/apis/Classes/HH/Awaitable/)s in the async
scheduler that they can work while this one waits for the blocking call to
finish (e.g., maybe in a polling situation or something).




## Returns




+ [` Awaitable<void> `](/apis/Classes/HH/Awaitable/) - [` Awaitable `](/apis/Classes/HH/Awaitable/) of `` void ``.
<!-- HHAPIDOC -->
