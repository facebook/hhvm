
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Compare and set







``` Hack
public function cas(
  int $cas,
  string $key,
  string $value,
  int $expiration = 0,
): Awaitable<void>;
```




## Parameters




+ ` int $cas ` - CAS token as returned by getRecord()
+ ` string $key ` - Name of the key to store
+ ` string $value ` - Datum to store
+ ` int $expiration = 0 `




## Returns




* [` Awaitable<void> `](/apis/Classes/HH/Awaitable/)
<!-- HHAPIDOC -->
