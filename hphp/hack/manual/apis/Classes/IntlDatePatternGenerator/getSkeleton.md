
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Utility to return a unique skeleton from a given pattern




``` Hack
public function getSkeleton(
  string $pattern,
): string;
```




For example, both "MMM-dd" and "dd/MMM" produce the skeleton "MMMdd".




## Parameters




+ ` string $pattern ` - Input pattern, such as "dd/MMM"




## Returns




* ` string ` - - skeleton such as "MMMdd"
<!-- HHAPIDOC -->
