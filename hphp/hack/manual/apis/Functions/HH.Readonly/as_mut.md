
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Converts a readonly value type into a mutable one




``` Hack
namespace HH\Readonly;

function as_mut<T>(
  T $x,
): T;
```




Value types include numerics, strings, bools, null and Hack arrays of value
types.




## Parameters




+ ` T $x `




## Returns




* ` T `
<!-- HHAPIDOC -->
