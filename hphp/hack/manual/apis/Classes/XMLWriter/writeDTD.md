
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Writes a full DTD




``` Hack
public function writeDTD(
  string $name,
  string $publicid = NULL,
  string $systemid = NULL,
  string $subset = NULL,
): bool;
```




## Parameters




+ ` string $name ` - The DTD name.
+ ` string $publicid = NULL ` - The external subset public identifier.
+ ` string $systemid = NULL ` - The external subset system identifier.
+ ` string $subset = NULL ` - The content of the DTD.




## Returns




* ` bool ` - - Returns TRUE on success or FALSE on failure.
<!-- HHAPIDOC -->
