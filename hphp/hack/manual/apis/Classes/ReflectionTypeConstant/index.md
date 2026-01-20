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




+ [` ::export($class, string $name, $return = false) `](/apis/Classes/ReflectionTypeConstant/export/)

+ [` ->__construct(mixed $class, string $name) `](/apis/Classes/ReflectionTypeConstant/__construct/)\
  Constructs a new ReflectionTypeConstant

+ [` ->__toString(): string `](/apis/Classes/ReflectionTypeConstant/__toString/)

+ [` ->getAssignedTypeText(): ?string `](/apis/Classes/ReflectionTypeConstant/getAssignedTypeText/)\
  Get the type assigned to this type constant as a string

+ [` ->getClass(): ReflectionClass `](/apis/Classes/ReflectionTypeConstant/getClass/)\
  Gets the class for the reflected type constant

+ [` ->getDeclaringClass(): ReflectionClass `](/apis/Classes/ReflectionTypeConstant/getDeclaringClass/)\
  Gets the declaring class for the reflected type constant

+ [` ->getName(): string `](/apis/Classes/ReflectionTypeConstant/getName/)\
  Get the name of the type constant

+ [` ->getTypeStructure(): darray `](/apis/Classes/ReflectionTypeConstant/getTypeStructure/)

+ [` ->isAbstract(): bool `](/apis/Classes/ReflectionTypeConstant/isAbstract/)\
  Checks if the type constant is abstract




<!-- HHAPIDOC -->
