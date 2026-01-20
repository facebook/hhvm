
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns a string representation of the given number with grouped thousands




``` Hack
namespace HH\Lib\Str;

function format_number(
  num $number,
  int $decimals = 0,
  string $decimal_point = '.',
  string $thousands_separator = ',',
): string;
```




If ` $decimals ` is provided, the string will contain that many decimal places.
The optional `` $decimal_point `` and ``` $thousands_separator ``` arguments define the
strings used for decimals and commas, respectively.




## Guide




+ [String](</hack/built-in-types/string>)







## Parameters




* ` num $number `
* ` int $decimals = 0 `
* ` string $decimal_point = '.' `
* ` string $thousands_separator = ',' `




## Returns




- ` string `
<!-- HHAPIDOC -->
