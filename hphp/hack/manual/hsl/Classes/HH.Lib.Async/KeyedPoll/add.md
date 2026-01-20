
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Add a single awaitable to the poll




``` Hack
public function add(
  Tk $key,
  Awaitable<Tv> $awaitable,
): void;
```




The key is retrieved with ` foreach ($poll await as $k => $v) {} `




## Parameters




+ ` Tk $key `
+ [` Awaitable<Tv> `](/apis/Classes/HH/Awaitable/)`` $awaitable ``




## Returns




* ` void `
<!-- HHAPIDOC -->
