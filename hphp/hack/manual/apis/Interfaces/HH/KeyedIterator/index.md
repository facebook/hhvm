---
title: KeyedIterator
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

For those entities that are [` KeyedTraversable `](/apis/Interfaces/HH/KeyedTraversable/), the `` KeyedIterator ``
interfaces provides the methods of iteration, included being able to get
the key




If a class implements ` KeyedIterator `, then it provides the infrastructure
to be iterated over using a `` foreach `` loop.




## Guides




+ [Introduction](</hack/arrays-and-collections/introduction>)
+ [Interfaces](</hack/arrays-and-collections/introduction>)







## Interface Synopsis




``` Hack
namespace HH;

interface KeyedIterator implements Iterator<Tv>, KeyedTraversable<Tk, Tv> {...}
```




### Public Methods




* [` ->key(): Tk `](/apis/Interfaces/HH/KeyedIterator/key/)\
  Return the current key at the current iterator position







### Public Methods ([` HH\Iterator `](/apis/Interfaces/HH/Iterator/))




- [` ->current(): Tv `](/apis/Interfaces/HH/Iterator/current/)\
  ( excerpt from http://php.net/manual/en/iterator.current.php )
- [` ->next(): void `](/apis/Interfaces/HH/Iterator/next/)\
  ( excerpt from http://php.net/manual/en/iterator.next.php )
- [` ->rewind(): void `](/apis/Interfaces/HH/Iterator/rewind/)\
  ( excerpt from http://php.net/manual/en/iterator.rewind.php )
- [` ->valid(): bool `](/apis/Interfaces/HH/Iterator/valid/)\
  ( excerpt from http://php.net/manual/en/iterator.valid.php )
<!-- HHAPIDOC -->
