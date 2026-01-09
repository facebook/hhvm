
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns whether a num is NAN




``` Hack
namespace HH\Lib\Math;

function is_nan(
  num $num,
): bool;
```




NAN is "the not-a-number special float value"




When comparing NAN to any value (including NAN) using operators
false will be returned. ` NAN === NAN ` is false.




One must always check for NAN using ` is_nan ` and not `` $x === NAN ``.




## Parameters




+ ` num $num `




## Returns




* ` bool `
<!-- HHAPIDOC -->
