
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Sets a hard breakpoint




``` Hack
function hphpd_break(
  bool $condition = true,
): void;
```




When a debugger is running, this line of code will
break into debugger, if condition is met. If there is no debugger that's
attached, it will not do anything.




## Parameters




+ ` bool $condition = true ` - If true, break, otherwise, continue.




## Returns




* ` void `
<!-- HHAPIDOC -->
