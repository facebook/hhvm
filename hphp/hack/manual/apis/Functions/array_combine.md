
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Creates an array by using the values from the keys array as keys and the
values from the values array as the corresponding values




``` Hack
function array_combine<Tv1 as arraykey, Tv2>(
  mixed $keys,
  mixed $values,
): darray<Tv1, Tv2>;
```




## Parameters




+ ` mixed $keys ` - Array of keys to be used. Illegal values for key will
  be converted to string.
+ ` mixed $values ` - Array of values to be used




## Returns




* ` mixed ` - - Returns the combined array, FALSE if the number of elements
  for each array isn't equal or if the arrays are empty.
<!-- HHAPIDOC -->
