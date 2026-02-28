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
namespace HH;

abstract final class Shapes {...}
```




### Public Methods




+ [` ::at(shape() $shape, arraykey $index) `](/apis/Classes/HH/Shapes/at/)\
  Returns the value of the field $index of $shape,
  throws if the field is missing
+ [` ::idx(shape(...) $shape, arraykey $index, mixed $default = NULL) `](/apis/Classes/HH/Shapes/idx/)\
  Use [` Shapes::idx `](/apis/Classes/HH/Shapes/idx/) to retrieve a field value in a shape, when the key may or may not exist
+ [` ::keyExists(shape() $shape, arraykey $index): bool `](/apis/Classes/HH/Shapes/keyExists/)\
  Check if a field in shape exists
+ [` ::removeKey<T as shape()>(inout darray $shape, arraykey $index): void `](/apis/Classes/HH/Shapes/removeKey/)\
  Removes the $index field from the $shape (passed in as an inout argument)
+ [` ::toArray(shape() $shape): darray<arraykey, mixed> `](/apis/Classes/HH/Shapes/toArray/)
+ [` ::toDict(shape() $shape): dict<arraykey, mixed> `](/apis/Classes/HH/Shapes/toDict/)
<!-- HHAPIDOC -->
