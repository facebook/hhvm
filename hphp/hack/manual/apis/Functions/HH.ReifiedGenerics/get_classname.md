
:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Returns the name of the class represented by this reified type




``` Hack
namespace HH\ReifiedGenerics;

function get_classname<T>(): classname<T>;
```




If this type does not represent a class, throws an exception




## Returns




+ ` classname<T> `
<!-- HHAPIDOC -->
