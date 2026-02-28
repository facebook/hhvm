
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Invokes a hard breakpoint




``` Hack
function hphp_debug_break(
  bool $condition = true,
): bool;
```




This routine will break into the debugger if
and only if the debugger is enabled and a debugger client is currently
attached.




## Parameters




+ ` bool $condition = true ` - Optional condition. If specified, the debugger
  will only break if the condition evaluates to true.




## Returns




* ` bool ` - - TRUE if the program successfully broke in (and has since
  resumed), FALSE if no debugger was attached.
<!-- HHAPIDOC -->
