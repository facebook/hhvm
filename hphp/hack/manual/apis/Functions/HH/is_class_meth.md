
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Checks whether the input is a native class method pointer




``` Hack
namespace HH;

function is_class_meth(
  mixed $arg,
): bool;
```




```
HH\is_class_meth(Foo::bar<>); // true
HH\is_class_meth(bing<>); // false
```




## Parameters




+ ` mixed $arg `




## Returns




* ` bool `
<!-- HHAPIDOC -->
