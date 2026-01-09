
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Fills an array with the value of the value parameter, using the values of
the keys array as keys




``` Hack
function array_fill_keys<Tk as arraykey, Tv>(
  mixed $keys,
  mixed $value,
): darray<arraykey, mixed, Tk, Tv>;
```




## Parameters




+ ` mixed $keys ` - Array of values that will be used as keys. Illegal
  values for key will be converted to string.
+ ` mixed $value ` - Value to use for filling




## Returns




* ` mixed ` - - Returns the filled array
<!-- HHAPIDOC -->
