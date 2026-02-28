
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The rules for coercion when doing a comparison where the RHS is an int
are complicated and it's not sufficient to just do straight casts on $value




``` Hack
namespace HH\Lib\Legacy_FIXME;

function int_cast_for_switch(
  mixed $value,
  ?arraykey $first_truthy = NULL,
): int;
```




Instead, we need to do some data tracking to convert the input to specifics
values to match specific cases under different circumstances.




arraykey instead of int second arg courtesy of non-transparent enums




## Parameters




+ ` mixed $value `
+ ` ?arraykey $first_truthy = NULL `




## Returns




* ` int `
<!-- HHAPIDOC -->
