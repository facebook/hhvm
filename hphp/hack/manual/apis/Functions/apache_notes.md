
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Set many apache request notes at once




``` Hack
function apache_notes(
  dict<string> $notes,
): void;
```




All keys must be strings.
All values must be strings.
Nullable values may not be used to remove keys as with apache_note().
This function returns nothing.




## Parameters




+ ` dict<string> $notes `




## Returns




* ` void `
<!-- HHAPIDOC -->
