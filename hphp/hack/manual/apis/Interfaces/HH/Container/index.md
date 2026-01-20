---
title: Container
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Represents an entity that can be iterated over using ` foreach `, without
requiring a key, except it does not include objects that implement
[` Iterator `](/apis/Interfaces/HH/Iterator/)




The iteration variable will have a type of ` T `.




In addition to Hack collections, PHP ` array `s are `` Container ``s.




## Guides




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Interfaces](</hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
namespace HH;

interface Container implements Traversable<Tv> {...}
```



<!-- HHAPIDOC -->
