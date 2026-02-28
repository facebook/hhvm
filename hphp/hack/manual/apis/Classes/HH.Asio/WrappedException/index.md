---
title: WrappedException
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Represents the result of failed [` Awaitable `](/apis/Classes/HH/Awaitable/) operation




## Interface Synopsis




``` Hack
namespace HH\Asio;

final class WrappedException implements ResultOrExceptionWrapper<Tr> {...}
```




### Public Methods




+ [` ->__construct(Te $exception) `](/apis/Classes/HH.Asio/WrappedException/__construct/)\
  Instantiate a `` WrappedException ``
+ [` ->getException(): Te `](/apis/Classes/HH.Asio/WrappedException/getException/)\
  Since this is a failed result wrapper, this always returns the exception
  thrown during the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation
+ [` ->getResult(): Tr `](/apis/Classes/HH.Asio/WrappedException/getResult/)\
  Since this is a failed result wrapper, this always returns the exception
  thrown during the [` Awaitable `](/apis/Classes/HH/Awaitable/) operation
+ [` ->isFailed(): bool `](/apis/Classes/HH.Asio/WrappedException/isFailed/)\
  Since this is a failed result wrapper, this always returns `` true ``
+ [` ->isSucceeded(): bool `](/apis/Classes/HH.Asio/WrappedException/isSucceeded/)\
  Since this is a failed result wrapper, this always returns `` false ``
<!-- HHAPIDOC -->
