---
title: AnyArray
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The parent class for all array types (containers that are values)




This currently includes both Hack Arrays (vec, dict, keyset) and Legacy
Arrays (varray, darray).




## Interface Synopsis




``` Hack
namespace HH;

abstract class AnyArray implements KeyedContainer<Tk, Tv>, \XHPChild {...}
```



<!-- HHAPIDOC -->
