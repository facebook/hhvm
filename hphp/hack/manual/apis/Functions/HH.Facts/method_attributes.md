
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get all attributes on the given method




``` Hack
namespace HH\Facts;

function method_attributes(
  classname<mixed> $type,
  string $method,
): vec<string, classname<\HH\MethodAttribute>>;
```




Return an empty vec if the method does not exist.




Throw InvalidOperationException if Facts is not enabled.




## Parameters




+ ` classname<mixed> $type `
+ ` string $method `




## Returns




* ` vec<string, classname<\HH\MethodAttribute>> `
<!-- HHAPIDOC -->
