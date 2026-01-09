
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Do a modification on the switched value




``` Hack
namespace HH\Lib\Legacy_FIXME;

function string_cast_for_basic_switch(
  mixed $value,
  ?arraykey $first_case,
): string;
```




This is in the case where none
of the case options are falsy, intish, or floatish




arraykey instead of string for second arg courtesy of non-transparent enums




## Parameters




+ ` mixed $value `
+ ` ?arraykey $first_case `




## Returns




* ` string `
<!-- HHAPIDOC -->
