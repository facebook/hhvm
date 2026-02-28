---
title: Shapes
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

## Interface Synopsis




``` Hack
namespace HH\Readonly;

abstract final class Shapes {...}
```




### Public Methods




+ [` ::at(shape() $shape, arraykey $index): mixed `](/apis/Classes/HH.Readonly/Shapes/at/)\
  Returns the value of the field $index of a readonly $shape,
  throws if the field is missing
+ [` ::idx(shape(...) $shape, arraykey $index, mixed $default = NULL): mixed `](/apis/Classes/HH.Readonly/Shapes/idx/)\
  Use [` Readonly\Shapes::idx `](/apis/Classes/HH.Readonly/Shapes/idx/) to retrieve a field value in a shape, when the key may or may not exist
+ [` ::toArray(shape() $shape): darray<arraykey, mixed> `](/apis/Classes/HH.Readonly/Shapes/toArray/)
+ [` ::toDict(shape() $shape): dict<arraykey, mixed> `](/apis/Classes/HH.Readonly/Shapes/toDict/)
<!-- HHAPIDOC -->
