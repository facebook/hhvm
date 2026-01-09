
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the overflow part of multiplying two ints plus another int, as if
they were all unsigned




``` Hack
namespace HH;

function int_mul_add_overflow(
  int $a,
  int $b,
  int $bias,
): int;
```




Specifically, this returns the upper 64 bits of
full (unsigned)$a * (unsigned)$b + (unsigned)$bias. $bias can be used to
manipulate rounding of the result.




## Parameters




+ ` int $a `
+ ` int $b `
+ ` int $bias `




## Returns




* ` int `
<!-- HHAPIDOC -->
