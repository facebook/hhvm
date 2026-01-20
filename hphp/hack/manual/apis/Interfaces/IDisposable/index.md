---
title: IDisposable
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Objects that implement IDisposable may be used in using statements




## Interface Synopsis




``` Hack
interface IDisposable {...}
```




### Public Methods




+ [` ->__dispose(): void `](/apis/Interfaces/IDisposable/__dispose/)\
  This method is invoked exactly once at the end of the scope of the
  using statement, unless the program terminates with a fatal error
<!-- HHAPIDOC -->
