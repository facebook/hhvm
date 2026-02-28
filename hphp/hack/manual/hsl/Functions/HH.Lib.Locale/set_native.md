
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Set the libc locale for the current thread




``` Hack
namespace HH\Lib\Locale;

function set_native(
  Locale $loc,
): void;
```




This is highly discouraged; see the note for ` get_native() ` for details.




## Parameters




+ ` Locale $loc `




## Returns




* ` void `
<!-- HHAPIDOC -->
