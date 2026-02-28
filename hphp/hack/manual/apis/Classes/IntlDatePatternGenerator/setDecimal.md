
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The decimal value is used in formatting fractions of seconds




``` Hack
public function setDecimal(
  string $decimal,
): void;
```




If the skeleton contains fractional seconds, then this is used with the
fractional seconds. For example, suppose that the input pattern is
"hhmmssSSSS", and the best matching pattern internally is "H:mm:ss", and
the decimal string is ",". Then the resulting pattern is modified to be
"H:mm:ss,SSSS"




## Parameters




+ ` string $decimal ` - The string to represent the decimal




## Returns




* ` void `
<!-- HHAPIDOC -->
