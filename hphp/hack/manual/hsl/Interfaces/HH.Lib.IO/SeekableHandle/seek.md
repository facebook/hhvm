
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Move to a specific offset within a handle




``` Hack
public function seek(
  int $offset,
): void;
```




Offset is relative to the start of the handle - so, the beginning of the
handle is always offset 0.




## Parameters




+ ` int $offset `




## Returns




* ` void `
<!-- HHAPIDOC -->
