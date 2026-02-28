---
title: IAsyncDisposable
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Objects that implement IAsyncDisposable may be used in await using statements




## Interface Synopsis




``` Hack
interface IAsyncDisposable {...}
```




### Public Methods




+ [` ->__disposeAsync(): Awaitable<void> `](/apis/Interfaces/IAsyncDisposable/__disposeAsync/)\
  This method is invoked exactly once at the end of the scope of the
  await using statement, unless the program terminates with a fatal error
<!-- HHAPIDOC -->
