---
title: PairIterator
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

## Interface Synopsis




``` Hack
final class PairIterator implements HH\KeyedIterator<int, mixed> {...}
```




### Public Methods




+ [` ->__construct(): void `](/apis/Classes/PairIterator/__construct/)
+ [` ->current(): mixed `](/apis/Classes/PairIterator/current/)\
  Returns the current value that the iterator points to
+ [` ->key(): int `](/apis/Classes/PairIterator/key/)\
  Returns the current key that the iterator points to
+ [` ->next(): void `](/apis/Classes/PairIterator/next/)\
  Advance this iterator forward one position
+ [` ->rewind(): void `](/apis/Classes/PairIterator/rewind/)\
  Move this iterator back to the first position
+ [` ->valid(): bool `](/apis/Classes/PairIterator/valid/)\
  Returns true if the iterator points to a valid value, returns false
  otherwise
<!-- HHAPIDOC -->
