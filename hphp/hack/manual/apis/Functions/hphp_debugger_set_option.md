
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Customizes the behavior of the debugger by setting an option flag on or off




``` Hack
function hphp_debugger_set_option(
  string $option,
  bool $value,
): bool;
```




## Parameters




+ ` string $option `
+ ` bool $value `




## Returns




* ` bool ` - - New value of the debugger option
<!-- HHAPIDOC -->
