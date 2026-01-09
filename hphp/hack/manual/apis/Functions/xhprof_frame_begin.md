
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Starts an artificial frame




``` Hack
function xhprof_frame_begin(
  string $name,
): void;
```




Together with xhprof_frame_end(), this times
one block of code execution as if it were a function call, allowing people
to define arbitrary function boundaries.




## Parameters




+ ` string $name ` - The "virtual" function's name.




## Returns




* ` void `
<!-- HHAPIDOC -->
