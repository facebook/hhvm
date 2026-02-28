
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get all attributes on the given type




``` Hack
namespace HH\Facts;

function type_attributes(
  classname<mixed> $type,
): vec<string, classname<\HH\ClassLikeAttribute>>;
```




Return an empty vec if the given type doesn't exist or is defined in more
than one file.




Throw InvalidOperationException if Facts is not enabled.




## Parameters




+ ` classname<mixed> $type `




## Returns




* ` vec<string, classname<\HH\ClassLikeAttribute>> `
<!-- HHAPIDOC -->
