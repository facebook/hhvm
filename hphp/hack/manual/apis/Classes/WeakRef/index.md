---
title: WeakRef
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

( excerpt from
http://php.net/manual/en/class.weakref.php )




The WeakRef class provides a gateway to objects without preventing the
garbage collector from freeing those objects. It also provides a way to turn
a weak reference into a strong one.




## Interface Synopsis




``` Hack
final class WeakRef {...}
```




### Public Methods




+ [` ->__construct(?T $reference) `](/docs/apis/Classes/WeakRef/__construct/)\
  ( excerpt from
  http://php.net/manual/en/weakref.construct.php )
+ [` ->acquire(): bool `](/docs/apis/Classes/WeakRef/acquire/)\
  ( excerpt from
  http://php.net/manual/en/weakref.acquire.php )
+ [` ->get(): ?T `](/docs/apis/Classes/WeakRef/get/)\
  ( excerpt from
  http://php.net/manual/en/weakref.get.php )
+ [` ->release(): bool `](/docs/apis/Classes/WeakRef/release/)\
  ( excerpt from
  http://php.net/manual/en/weakref.release.php )
+ [` ->valid(): bool `](/docs/apis/Classes/WeakRef/valid/)\
  ( excerpt from
  http://php.net/manual/en/weakref.valid.php )
<!-- HHAPIDOC -->
