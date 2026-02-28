---
title: Traversable
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Represents an entity that can be iterated over using ` foreach `, without
requiring a key




The iteration variable will have a type of ` T `.




In addition to Hack collections, PHP ` array `s and anything that implement
[` Iterator `](/apis/Interfaces/HH/Iterator/) are `` Traversable ``.




In general, if you are implementing your own Hack class, you will want to
implement [` Iterable `](/apis/Interfaces/HH/Iterable/) instead of `` Traversable `` since ``` Traversable ``` is more
of a bridge for PHP ```` array ````s to work well with Hack collections.




## Guides




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Interfaces](</hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
namespace HH;

interface Traversable {...}
```



<!-- HHAPIDOC -->
