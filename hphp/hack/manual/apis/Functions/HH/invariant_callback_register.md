
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Pass a function that will be called if any ` invariant ` fails




``` Hack
namespace HH;

function invariant_callback_register(
  (function(string, mixed ...): void) $callback,
): void;
```




The callback
will be called with all the invariant parameters after the condition.




## Parameters




+ ` (function(string, mixed ...): void) $callback ` - The function that will be called if your invariant fails.




## Returns




* ` void `
<!-- HHAPIDOC -->
