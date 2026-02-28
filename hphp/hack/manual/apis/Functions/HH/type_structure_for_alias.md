
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Retrieves the TypeStructure for a type alias




``` Hack
namespace HH;

function type_structure_for_alias<T>(
  typename<T> $cls_or_obj,
): TypeStructure<T>;
```




## Parameters




+ ` typename<T> $cls_or_obj `




## Returns




* ` darray ` - The resolved type structure for a type alias.
<!-- HHAPIDOC -->
