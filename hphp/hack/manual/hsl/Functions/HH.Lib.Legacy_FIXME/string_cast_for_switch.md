
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The rules for coercion when doing a comparison where the RHS is a string
are complicated and it's not sufficient to just do straight casts on $value




``` Hack
namespace HH\Lib\Legacy_FIXME;

function string_cast_for_switch(
  mixed $value,
  ?arraykey $first_truthy = NULL,
  ?arraykey $first_zeroish = NULL,
  ?arraykey $first_falsy = NULL,
  dict<arraykey, int> $intish_vals = dict [
],
  dict<arraykey, float> $floatish_vals = dict [
],
): string;
```




Instead, we need to do some data tracking to convert the input to specifics
values to match specific cases under different circumstances.




arraykey instead of string for second arg courtesy of non-transparent enums *




## Parameters




+ ` mixed $value `
+ ` ?arraykey $first_truthy = NULL `
+ ` ?arraykey $first_zeroish = NULL `
+ ` ?arraykey $first_falsy = NULL `
+ ` dict<arraykey, int> $intish_vals = dict [ ] `
+ ` dict<arraykey, float> $floatish_vals = dict [ ] `




## Returns




* ` string `
<!-- HHAPIDOC -->
