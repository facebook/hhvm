
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

``` Hack
namespace HH;

function str_to_numeric(
  string $str,
): ?num;
```




## Parameters




+ ` string $str ` - The input string




## Returns




* ` ?num ` - - If $str is not "numeric" as per the definition of
  `` str_number_coercible `` null is returned. Otherwise, return the value of the
  string coerced to a number
<!-- HHAPIDOC -->
