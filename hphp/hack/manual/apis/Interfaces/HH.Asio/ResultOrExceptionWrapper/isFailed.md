
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Indicates whether the [` Awaitable `](/apis/Classes/HH/Awaitable/) associated with this wrapper exited
abnormally via an exception of somoe sort




``` Hack
public function isFailed(): bool;
```




If [` isFailed() `](/apis/Interfaces/HH.Asio/ResultOrExceptionWrapper/isFailed/) returns `` true ``, [` isSucceeded() `](/apis/Interfaces/HH.Asio/ResultOrExceptionWrapper/isSucceeded/) returns `` false ``.




## Returns




+ ` bool ` - `` true `` if the [` Awaitable `](/apis/Classes/HH/Awaitable/) failed; `` false `` otherwise.
<!-- HHAPIDOC -->
