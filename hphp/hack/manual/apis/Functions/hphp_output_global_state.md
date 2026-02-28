
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Dumps all variables in global state, including global variables, static
variables, class statics and others




``` Hack
function hphp_output_global_state(
  bool $serialize = true,
): mixed;
```




## Parameters




+ ` bool $serialize = true ` - Specifies what format to use, whether to serialize
  into a string.




## Returns




* ` mixed ` - - An array of global state.
<!-- HHAPIDOC -->
