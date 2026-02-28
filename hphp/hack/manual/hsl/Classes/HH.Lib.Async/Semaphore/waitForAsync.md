
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Produce a ` Tout ` from a `` Tin ``, respecting the concurrency limit




``` Hack
public function waitForAsync(
  Tin $value,
): Awaitable<Tout>;
```




## Parameters




+ ` Tin $value `




## Returns




* [` Awaitable<Tout> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
