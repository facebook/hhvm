
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Starts a DTD




``` Hack
public function startDTD(
  string $qualifiedname,
  string $publicid = NULL,
  string $systemid = NULL,
): bool;
```




## Parameters




+ ` string $qualifiedname ` - The qualified name of the document type to
  create.
+ ` string $publicid = NULL ` - The external subset public identifier.
+ ` string $systemid = NULL ` - The external subset system identifier.




## Returns




* ` bool ` - - Returns TRUE on success or FALSE on failure.
<!-- HHAPIDOC -->
