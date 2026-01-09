---
title: ReflectionTypeConstant
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The ReflectionTypeConstant class reports information about an object




## Interface Synopsis




``` Hack
class ReflectionTypeConstant implements Reflector {...}
```




### Public Methods




+ [` ::export($class, string $name, $return = false) `](/docs/apis/Classes/ReflectionTypeConstant/export/)

+ [` ->__construct(mixed $class, string $name) `](/docs/apis/Classes/ReflectionTypeConstant/__construct/)\
  Constructs a new ReflectionTypeConstant

+ [` ->__toString(): string `](/docs/apis/Classes/ReflectionTypeConstant/__toString/)

+ [` ->getAssignedTypeText(): ?string `](/docs/apis/Classes/ReflectionTypeConstant/getAssignedTypeText/)\
  Get the type assigned to this type constant as a string

+ [` ->getClass(): ReflectionClass `](/docs/apis/Classes/ReflectionTypeConstant/getClass/)\
  Gets the class for the reflected type constant

+ [` ->getDeclaringClass(): ReflectionClass `](/docs/apis/Classes/ReflectionTypeConstant/getDeclaringClass/)\
  Gets the declaring class for the reflected type constant

+ [` ->getName(): string `](/docs/apis/Classes/ReflectionTypeConstant/getName/)\
  Get the name of the type constant

+ [` ->getTypeStructure(): darray `](/docs/apis/Classes/ReflectionTypeConstant/getTypeStructure/)

+ [` ->isAbstract(): bool `](/docs/apis/Classes/ReflectionTypeConstant/isAbstract/)\
  Checks if the type constant is abstract




<!-- HHAPIDOC -->
