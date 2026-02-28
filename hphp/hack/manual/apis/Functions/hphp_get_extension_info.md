
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

hphp_get_extension_info() - Internally used for getting extension's
information




``` Hack
function hphp_get_extension_info(
  string $name,
): darray<string, mixed>;
```




## Parameters




+ ` string $name ` - Name of the extension




## Returns




* ` array ` - - A map containing the extension's name, version, info string
  ini settings, constants, functions and classes.
<!-- HHAPIDOC -->
