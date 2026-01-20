---
title: KeyedTraversable
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Represents an entity that can be iterated over using ` foreach `, allowing
a key




The iteration variables will have a type of ` Tk ` for the key and `` Tv `` for the
value.




In addition to Hack collections, PHP ` array `s and anything that implement
[` KeyedIterator `](/apis/Interfaces/HH/KeyedIterator/) are `` KeyedTraversable ``.




In general, if you are implementing your own Hack class, you will want to
implement [` KeyedIterable `](/apis/Interfaces/HH/KeyedIterable/) instead of `` KeyedTraversable `` since
``` KeyedTraversable ``` is more of a bridge for PHP ```` array ````s to work well with
Hack collections.




## Guides




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Interfaces](</hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
namespace HH;

interface KeyedTraversable implements Traversable<Tv> {...}
```



<!-- HHAPIDOC -->
