---
title: Ref
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Wrapper class for getting object (byref) semantics for a value type




This is especially useful for mutating values outside of a lambda's scope.




In general, it's preferable to refactor to use return values or ` inout `
parameters instead of using this class - however, a `` Ref `` of a Hack array
is generally preferable to a Hack collection - e.g. prefer [` Ref<vec<T>> `](/hsl/Classes/HH.Lib/Ref/)
over [` Vector<T> `](/apis/Classes/HH/Vector/).




` C\reduce() ` and `` C\reduce_with_key() `` can also be used in some situations
to avoid this class.




## Interface Synopsis




``` Hack
namespace HH\Lib;

final class Ref {...}
```




### Public Methods




+ [` ->__construct(T $value) `](/hsl/Classes/HH.Lib/Ref/__construct/)
+ [` ->get(): T `](/hsl/Classes/HH.Lib/Ref/get/)\
  Retrieve the stored value
+ [` ->set(T $new_value): void `](/hsl/Classes/HH.Lib/Ref/set/)\
  Set the new value
<!-- HHAPIDOC -->
