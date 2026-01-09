
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Toggles the compression status of HipHop output, if headers have already
been sent this may be ignored




``` Hack
function fb_output_compression(
  bool $new_value,
): bool;
```




## Parameters




+ ` bool $new_value ` - The new value for the compression state.




## Returns




* ` bool ` - - The old value.
<!-- HHAPIDOC -->
