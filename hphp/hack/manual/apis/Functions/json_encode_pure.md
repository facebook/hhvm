
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Like json_encode_with_error but has pure coeffects




``` Hack
function json_encode_pure(
  mixed $value,
  inout ?(int, string) $error,
  int $options = 0,
  int $depth = 512,
): mixed;
```




Encoding objects implementing JsonSerializable with an impure jsonSerialize
will result in coeffect violations.




## Parameters




+ ` mixed $value `
+ ` inout ?(int, string) $error `
+ ` int $options = 0 `
+ ` int $depth = 512 `




## Returns




* ` mixed `
<!-- HHAPIDOC -->
