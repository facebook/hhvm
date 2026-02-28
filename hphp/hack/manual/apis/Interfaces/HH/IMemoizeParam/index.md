---
title: IMemoizeParam
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

Classes that implement IMemoizeParam may be used as parameters on
`<<__Memoize>>` functions







## Guides




+ [Introduction](</hack/attributes/introduction>)
+ [Special](</hack/attributes/predefined-attributes>)







## Interface Synopsis




``` Hack
namespace HH;

interface IMemoizeParam {...}
```




### Public Methods




* [` ->getInstanceKey(): string `](/apis/Interfaces/HH/IMemoizeParam/getInstanceKey/)\
  Serialize this object to a string that can be used as a
  dictionary key to differentiate instances of this class
<!-- HHAPIDOC -->
