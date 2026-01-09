
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Utility to return a unique base skeleton from a given pattern




``` Hack
public function getBaseSkeleton(
  string $pattern,
): string;
```




This is the same as the skeleton, except that differences in length are
minimized so as to only preserve the difference between string and numeric
form. So for example, both "MMM-dd" and "d/MMM" produce the skeleton "MMMd"
(notice the single d).




## Parameters




+ ` string $pattern ` - Input pattern, such as "dd/MMM"




## Returns




* ` string ` - - base skeleton, such as "Md"
<!-- HHAPIDOC -->
