
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Wrap an [` Awaitable `](/apis/Classes/HH/Awaitable/) into an [` Awaitable `](/apis/Classes/HH/Awaitable/) of `` ResultOrExceptionWrapper ``




``` Hack
namespace HH\Asio;

function wrap<Tv>(
  Awaitable<Tv> $awaitable,
): Awaitable<ResultOrExceptionWrapper<Tv>>;
```




The actual ` ResultOrExceptionWrapper ` in the returned [` Awaitable `](/apis/Classes/HH/Awaitable/) will only
be available after you `` await `` or ``` join ``` the returned [` Awaitable `](/apis/Classes/HH/Awaitable/).




## Parameters




+ [` Awaitable<Tv> `](/apis/Classes/HH/Awaitable/)`` $awaitable `` - The [` Awaitable `](/apis/Classes/HH/Awaitable/) to wrap.




## Returns




* [` Awaitable<ResultOrExceptionWrapper<Tv>> `](/apis/Classes/HH/Awaitable/) - the [` Awaitable `](/apis/Classes/HH/Awaitable/) of `` ResultOrExceptionWrapper ``.
<!-- HHAPIDOC -->
