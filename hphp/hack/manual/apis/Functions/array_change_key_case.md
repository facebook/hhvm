
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns an array with all keys from input lowercased or uppercased




``` Hack
function array_change_key_case<Tv>(
  mixed $input,
  int $upper = CASE_LOWER,
): darray<arraykey, Tv>;
```




Numbered indices are left as is.




## Parameters




+ ` mixed $input ` - The array to work on
+ ` int $upper = CASE_LOWER `




## Returns




* ` mixed ` - - Returns an array with its keys lower or uppercased, or
  FALSE if input is not an array.
<!-- HHAPIDOC -->
