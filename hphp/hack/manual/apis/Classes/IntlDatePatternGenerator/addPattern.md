
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Adds a pattern to the generator




``` Hack
public function addPattern(
  string $pattern,
  bool $override,
): int;
```




If the pattern has the same skeleton as an existing pattern, and the
override parameter is set, then the previous value is overridden.
Otherwise, the previous value is retained.
Note that single-field patterns (like "MMM") are automatically added, and
don't need to be added explicitly!




## Parameters




+ ` string $pattern ` - Input pattern, such as "dd/MMM"
+ ` bool $override ` - When existing values are to be overridden use true,
  otherwise use false.




## Returns




* ` int ` - - pattern conflict status (see constants)
<!-- HHAPIDOC -->
