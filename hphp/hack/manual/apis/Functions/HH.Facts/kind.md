
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Return a string enum representing whether the given type is, for example, a
class, interface, or trait




``` Hack
namespace HH\Facts;

function kind(
  classname<mixed> $type,
): ?TypeKind;
```




If the given type doesn't have a unique definition or isn't a
` classname<mixed> `, return `` null ``.




## Parameters




+ ` classname<mixed> $type `




## Returns




* ` ?TypeKind `
<!-- HHAPIDOC -->
