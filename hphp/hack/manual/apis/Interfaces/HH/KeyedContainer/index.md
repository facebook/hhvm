---
title: KeyedContainer
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

` KeyedContainer ` allows you to do `` (foreach $value as $k => $v, $value[$key]) `` and is an
interface used by both Hack arrays (vec/dict/keyset) and Hack collections (Vector/Map/Set)




If you need to iterate over an array / collection with keys, use KeyedContainer; otherwise, use Container.
Without iterating, you can access keys directly: ` $keyed_container[$key] `.




## Guides




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Interfaces](</hack/arrays-and-collections/introduction>)
+ [Read Write](</hack/arrays-and-collections/mutating-values>)







## Interface Synopsis




``` Hack
namespace HH;

interface KeyedContainer implements Container<Tv>, KeyedTraversable<Tk, Tv> {...}
```



<!-- HHAPIDOC -->
