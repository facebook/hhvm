
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

preg_match, but populates $error in case of error




``` Hack
function preg_match_with_error(
  string $pattern,
  string $subject,
  inout ?int $error,
  int $flags = 0,
  int $offset = 0,
): int;
```




If the function runs normally with no errors, then $error is set to null.
Otherwise, if an error occurs, $error is set to an error code constant from
the list defined in builtins_preg.hhi.




## Parameters




+ ` string $pattern `
+ ` string $subject `
+ ` inout ?int $error `
+ ` int $flags = 0 `
+ ` int $offset = 0 `




## Returns




* ` int `
<!-- HHAPIDOC -->
