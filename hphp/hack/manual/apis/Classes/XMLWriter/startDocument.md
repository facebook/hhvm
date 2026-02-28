
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Starts a document




``` Hack
public function startDocument(
  string $version = '1.0',
  string $encoding = NULL,
  string $standalone = NULL,
): bool;
```




## Parameters




+ ` string $version = '1.0' ` - The version number of the document as part of the
  XML declaration. Defaults to 1.0.
+ ` string $encoding = NULL ` - The encoding of the document as part of the XML
  declaration. NULL by default.
+ ` string $standalone = NULL ` - yes or no.




## Returns




* ` bool ` - - Returns TRUE on success or FALSE on failure.
<!-- HHAPIDOC -->
