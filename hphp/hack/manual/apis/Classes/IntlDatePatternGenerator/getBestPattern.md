
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the best pattern matching the input skeleton




``` Hack
public function getBestPattern(
  string $skeleton,
): string;
```




It is guaranteed to have all of the fields in the skeleton.




## Parameters




+ ` string $skeleton ` - The skeleton is a pattern containing only the
  variable fields. For example, "MMMdd" and "mmhh" are skeletons.




## Returns




* ` string ` - - The best pattern found for the given skeleton
<!-- HHAPIDOC -->
