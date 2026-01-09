
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Writes a full DTD entity




``` Hack
public function writeDTDEntity(
  string $name,
  string $content,
  bool $pe = false,
  string $publicid = '',
  string $systemid = '',
  string $ndataid = '',
): bool;
```




## Parameters




+ ` string $name ` - The name of the entity.
+ ` string $content ` - The content of the entity.
+ ` bool $pe = false `
+ ` string $publicid = '' `
+ ` string $systemid = '' `
+ ` string $ndataid = '' `




## Returns




* ` bool ` - - Returns TRUE on success or FALSE on failure.
<!-- HHAPIDOC -->
