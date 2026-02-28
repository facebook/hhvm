
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Set a callback function that is called when php tries to exit




``` Hack
function fb_set_exit_callback(
  mixed $function,
): void;
```




## Parameters




+ ` mixed $function ` - The callback to invoke. An exception object will
  be passed to the function




## Returns




* ` void `
<!-- HHAPIDOC -->
