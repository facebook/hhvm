
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Sets the name of a field, eg "era" in English for ERA




``` Hack
public function setAppendItemName(
  int $field,
  string $name,
): void;
```




These are only used if the corresponding AppendItemFormat is used, and if
it contains a {2} variable.




## Parameters




+ ` int $field ` - Pattern field (see constants)
+ ` string $name ` - Name of the field




## Returns




* ` void `
<!-- HHAPIDOC -->
