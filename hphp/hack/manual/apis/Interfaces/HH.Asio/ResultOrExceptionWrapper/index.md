---
title: ResultOrExceptionWrapper
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Represents a result of operation that either has a successful result of an
[` Awaitable `](/apis/Classes/HH/Awaitable/) or the exception object if that [` Awaitable `](/apis/Classes/HH/Awaitable/) failed




This is an interface. You get generally ` ResultOrExceptionWrapper ` by calling
`` wrap() ``, passing in the [` Awaitable `](/apis/Classes/HH/Awaitable/), and a `` WrappedResult `` or
``` WrappedException ``` is returned.




## Interface Synopsis




``` Hack
namespace HH\Asio;

interface ResultOrExceptionWrapper {...}
```




### Public Methods




+ [` ->getException(): \Exception `](/apis/Interfaces/HH.Asio/ResultOrExceptionWrapper/getException/)\
  Return the underlying exception, or fail with invariant violation
+ [` ->getResult(): T `](/apis/Interfaces/HH.Asio/ResultOrExceptionWrapper/getResult/)\
  Return the result of the operation, or throw underlying exception
+ [` ->isFailed(): bool `](/apis/Interfaces/HH.Asio/ResultOrExceptionWrapper/isFailed/)\
  Indicates whether the [` Awaitable `](/apis/Classes/HH/Awaitable/) associated with this wrapper exited
  abnormally via an exception of somoe sort
+ [` ->isSucceeded(): bool `](/apis/Interfaces/HH.Asio/ResultOrExceptionWrapper/isSucceeded/)\
  Indicates whether the [` Awaitable `](/apis/Classes/HH/Awaitable/) associated with this wrapper exited
  normally
<!-- HHAPIDOC -->
