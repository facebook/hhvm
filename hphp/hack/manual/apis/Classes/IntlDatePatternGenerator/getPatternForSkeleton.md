
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get the pattern corresponding to a given skeleton




``` Hack
public function getPatternForSkeleton(
  string $skeleton,
): string;
```




## Parameters




+ ` string $skeleton ` - The skeleton is a pattern containing only the
  variable fields. For example, "MMMdd" and "mmhh" are skeletons.




## Returns




* ` string ` - - pattern
<!-- HHAPIDOC -->
