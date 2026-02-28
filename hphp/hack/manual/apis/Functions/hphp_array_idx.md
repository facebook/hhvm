
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

hphp_array_idx() returns the value at the given key in the given array or
the given default value if it is not found




``` Hack
function hphp_array_idx(
  mixed $search,
  mixed $key,
  mixed $def,
): mixed;
```




An error will be raised if the
search parameter is not an array.




## Parameters




+ ` mixed $search ` - An array with keys to check.
+ ` mixed $key ` - Value to check.
+ ` mixed $def ` - The value to return if key is not found in search.




## Returns




* ` mixed ` - - Returns the value at 'key' in 'search' or 'def' if it is
  not found.
<!-- HHAPIDOC -->
