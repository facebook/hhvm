
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

An append item format is a pattern used to append a field if there is no
good match




``` Hack
public function setAppendItemFormat(
  int $field,
  string $value,
): void;
```




For example, suppose that the input skeleton is "GyyyyMMMd", and there is
no matching pattern internally, but there is a pattern matching "yyyyMMMd",
say "d-MM-yyyy". Then that pattern is used, plus the G. The way these two
are conjoined is by using the AppendItemFormat for G (era). So if that
value is, say "{0}, {1}" then the final resulting pattern is
"d-MM-yyyy, G".




There are actually three available variables: {0} is the pattern so far,
{1} is the element we are adding, and {2} is the name of the element.




## Parameters




+ ` int $field ` - Pattern field (see constants)
+ ` string $value ` - Pattern, such as "{0}, {1}"




## Returns




* ` void `
<!-- HHAPIDOC -->
