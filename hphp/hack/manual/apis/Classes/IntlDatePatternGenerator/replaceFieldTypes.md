
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Adjusts the field types (width and subtype) of a pattern to match what is
in a skeleton




``` Hack
public function replaceFieldTypes(
  string $pattern,
  string $skeleton,
): string;
```




Example: given an input pattern of "d-M H:m", and a skeleton of
"MMMMddhhmm", the output pattern will be "dd-MMMM hh:mm".




## Parameters




+ ` string $pattern ` - Input pattern
+ ` string $skeleton ` - The skeleton is a pattern containing only the
  variable fields. For example, "MMMdd" and "mmhh" are skeletons.




## Returns




* ` string ` - - Pattern adjusted to match the skeleton fields widths and
  subtypes.
<!-- HHAPIDOC -->
