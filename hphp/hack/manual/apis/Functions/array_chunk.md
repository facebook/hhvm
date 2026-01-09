
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Chunks an array into size large chunks




``` Hack
function array_chunk<Tv>(
  mixed $input,
  int $size,
  bool $preserve_keys = false,
): varray<Container<int>>;
```




The last chunk may contain less
than size elements.




## Parameters




+ ` mixed $input ` - The array to work on
+ ` int $size ` - The size of each chunk
+ ` bool $preserve_keys = false ` - When set to TRUE keys will be preserved.
  Default is FALSE which will reindex the chunk numerically




## Returns




* ` mixed ` - - Returns a multidimensional numerically indexed array,
  starting with zero, with each dimension containing size elements.
<!-- HHAPIDOC -->
