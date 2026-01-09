
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

array_multisort() can be used to sort several arrays at once, or a
multi-dimensional array by one or more dimensions




``` Hack
function array_multisort1(
  inout mixed $arg1,
): bool;
```




Associative (string)
keys will be maintained, but numeric keys will be re-indexed.




## Parameters




+ ` inout mixed $arg1 ` - An array being sorted.




## Returns




* ` bool ` - - Returns TRUE on success or FALSE on failure.
<!-- HHAPIDOC -->
