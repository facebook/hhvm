
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

array_count_values() returns an array using the values of the input array
as keys and their frequency in input as values




``` Hack
function array_count_values<Tv as arraykey>(
  AnyArray $input,
): darray<Tv, int>;
```




## Parameters




+ [` AnyArray `](/apis/Classes/HH/AnyArray/)`` $input `` - The array of values to count




## Returns




* ` mixed ` - - Returns an associative array of values from input as keys
  and their count as value.
<!-- HHAPIDOC -->
