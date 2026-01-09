
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Compares two numbers to see if they are within epsilon of each other




``` Hack
namespace HH\Lib\Math;

function almost_equals(
  num $num_one,
  num $num_two,
  num $epsilon = 1.0E-8,
): bool;
```




If the difference equals epsilon this returns false.




default epsilon of .00000001.




When comparing large numbers consider passing in a large epsilon




## Parameters




+ ` num $num_one `
+ ` num $num_two `
+ ` num $epsilon = 1.0E-8 `




## Returns




* ` bool `
<!-- HHAPIDOC -->
