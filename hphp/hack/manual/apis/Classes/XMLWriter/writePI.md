
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Writes a processing instruction




``` Hack
public function writePI(
  string $target,
  string $content,
): bool;
```




## Parameters




+ ` string $target ` - The target of the processing instruction.
+ ` string $content ` - The content of the processing instruction.




## Returns




* ` bool ` - - Returns TRUE on success or FALSE on failure.
<!-- HHAPIDOC -->
