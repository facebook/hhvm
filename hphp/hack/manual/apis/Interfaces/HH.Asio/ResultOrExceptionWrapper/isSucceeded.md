
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Indicates whether the [` Awaitable `](/apis/Classes/HH/Awaitable/) associated with this wrapper exited
normally




``` Hack
public function isSucceeded(): bool;
```




If [` isSucceeded() `](/apis/Interfaces/HH.Asio/ResultOrExceptionWrapper/isSucceeded/) returns `` true ``, [` isFailed() `](/apis/Interfaces/HH.Asio/ResultOrExceptionWrapper/isFailed/) returns `` false ``.




## Returns




+ ` bool ` - `` true `` if the [` Awaitable `](/apis/Classes/HH/Awaitable/) succeeded; `` false `` otherwise.
<!-- HHAPIDOC -->
