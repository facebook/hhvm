
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Read a single byte from the handle




``` Hack
public function readByteAsync(
  ?int $timeout_ns = NULL,
): Awaitable<string>;
```




Fails with EPIPE if the handle is closed or otherwise unreadable.




## Parameters




+ ` ?int $timeout_ns = NULL `




## Returns




* [` Awaitable<string> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
