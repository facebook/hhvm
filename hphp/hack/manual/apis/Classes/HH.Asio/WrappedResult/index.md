---
title: WrappedResult
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Represents the result of successful [` Awaitable `](/apis/Classes/HH/Awaitable/) operation




## Interface Synopsis




``` Hack
namespace HH\Asio;

final class WrappedResult implements ResultOrExceptionWrapper<T> {...}
```




### Public Methods




+ [` ->__construct(T $result) `](/apis/Classes/HH.Asio/WrappedResult/__construct/)\
  Instantiate a `` WrappedResult ``
+ [` ->getException(): \Exception `](/apis/Classes/HH.Asio/WrappedResult/getException/)\
  Since this is a successful result wrapper, this always returns an
  [` InvariantException `](/apis/Classes/HH/InvariantException/) saying that there was no exception thrown from
  the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation
+ [` ->getResult(): T `](/apis/Classes/HH.Asio/WrappedResult/getResult/)\
  Since this is a successful result wrapper, this always returns the actual
  result of the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation
+ [` ->isFailed(): bool `](/apis/Classes/HH.Asio/WrappedResult/isFailed/)\
  Since this is a successful result wrapper, this always returns `` false ``
+ [` ->isSucceeded(): bool `](/apis/Classes/HH.Asio/WrappedResult/isSucceeded/)\
  Since this is a successful result wrapper, this always returns `` true ``
<!-- HHAPIDOC -->
