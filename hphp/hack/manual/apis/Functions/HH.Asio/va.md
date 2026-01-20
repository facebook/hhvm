
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Translate a varargs of [` Awaitable `](/apis/Classes/HH/Awaitable/)s into a single [` Awaitable<(...)> `](/apis/Classes/HH/Awaitable/)




``` Hack
namespace HH\Asio;

function va(
  ...$awaitables,
): Awaitable;
```




This function's behavior cannot be expressed with type hints,
so it's hardcoded in the typechecker:




```
HH\Asio\va(Awaitable<T1>, Awaitable<T2>, ... , Awaitable<Tn>)
```




will return




```
Awaitable<(T1, T2, ..., Tn)>
```




## Parameters




+ ` ...$awaitables `




## Returns




* [` Awaitable `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
