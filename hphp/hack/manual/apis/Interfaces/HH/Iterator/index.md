---
title: Iterator
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

For those entities that are [` Traversable `](/apis/Interfaces/HH/Traversable/), the `` Iterator `` interfaces provides
the methods of iteration




If a class implements ` Iterator `, then it provides the infrastructure to be
iterated over using a `` foreach `` loop.




## Guides




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Interfaces](</hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
namespace HH;

interface Iterator implements Traversable<Tv> {...}
```




### Public Methods




* [` ->current(): Tv `](/apis/Interfaces/HH/Iterator/current/)\
  ( excerpt from http://php.net/manual/en/iterator.current.php )
* [` ->key(): mixed `](/apis/Interfaces/HH/Iterator/key/)\
  ( excerpt from http://php.net/manual/en/iterator.key.php )
* [` ->next(): void `](/apis/Interfaces/HH/Iterator/next/)\
  ( excerpt from http://php.net/manual/en/iterator.next.php )
* [` ->rewind(): void `](/apis/Interfaces/HH/Iterator/rewind/)\
  ( excerpt from http://php.net/manual/en/iterator.rewind.php )
* [` ->valid(): bool `](/apis/Interfaces/HH/Iterator/valid/)\
  ( excerpt from http://php.net/manual/en/iterator.valid.php )
<!-- HHAPIDOC -->
