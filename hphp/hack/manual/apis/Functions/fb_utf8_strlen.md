
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Count the number of UTF-8 code points in string, substituting U+FFFD for
invalid sequences




``` Hack
function fb_utf8_strlen(
  string $input,
): int;
```




## Parameters




+ ` string $input ` - The string.




## Returns




* ` int ` - - Returns the number of code points interpreting string as
  UTF-8.
<!-- HHAPIDOC -->
