
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Sanitize a string to make sure it's legal UTF-8 by stripping off any
characters that are not properly encoded




``` Hack
function fb_utf8ize(
  inout mixed $input,
): bool;
```




## Parameters




+ ` inout mixed $input ` - What string to sanitize.




## Returns




* ` bool ` - - Sanitized string.
<!-- HHAPIDOC -->
