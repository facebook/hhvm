
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Construct a cls_meth pointer for the method $cls::$meth




``` Hack
namespace HH;

function dynamic_class_meth(
  string $cls_name,
  string $meth_name,
): dynamic;
```




The method should be
a static method marked __DynamicallyCallable.




## Parameters




+ ` string $cls_name `
+ ` string $meth_name `




## Returns




* ` dynamic `
<!-- HHAPIDOC -->
