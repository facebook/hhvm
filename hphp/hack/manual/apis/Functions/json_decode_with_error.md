
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

json_decode, but populates $error in case of error




``` Hack
function json_decode_with_error(
  string $json,
  inout ?(int, string) $error,
  bool $assoc = false,
  int $depth = 512,
  int $options = 0,
): mixed;
```




If the function runs normally with no errors, then $error is set to null.
Otherwise, if an error occurs, $error is set to a tuple of (error code
constant, description) from the list defined in builtins_json.hhi.




## Parameters




+ ` string $json `
+ ` inout ?(int, string) $error `
+ ` bool $assoc = false `
+ ` int $depth = 512 `
+ ` int $options = 0 `




## Returns




* ` mixed `
<!-- HHAPIDOC -->
