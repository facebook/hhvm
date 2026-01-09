
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Get all parameters for the given attribute on the given method




``` Hack
namespace HH\Facts;

function method_attribute_parameters(
  classname<mixed> $type,
  string $method,
  classname<\HH\MethodAttribute> $attribute,
): vec<dynamic>;
```




Return an empty vec if the method doesn't exist or doesn't have the
given attribute.




Throw InvalidOperationException if Facts is not enabled.




## Parameters




+ ` classname<mixed> $type `
+ ` string $method `
+ ` classname<\HH\MethodAttribute> $attribute `




## Returns




* ` vec<dynamic> `
<!-- HHAPIDOC -->
