
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the overflow part of multiplying two ints, as if they were unsigned




``` Hack
namespace HH;

function int_mul_overflow(
  int $a,
  int $b,
): int;
```




In other words, this returns the upper 64 bits of the full product of
(unsigned)$a and (unsigned)$b. (The lower 64 bits is just ` $a * $b `
regardless of signed/unsigned).




## Parameters




+ ` int $a `
+ ` int $b `




## Returns




* ` int `
<!-- HHAPIDOC -->
