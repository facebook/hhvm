
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Converts a readonly value to into a mutable one but omits validation for the
purposes of performance




``` Hack
namespace HH\Readonly;

function as_mut_without_validation<T>(
  T $x,
): T;
```




This function takes advantage of non-enforcement of readonly is SystemLib
and is not safe.




## Parameters




+ ` T $x `




## Returns




* ` T `
<!-- HHAPIDOC -->
