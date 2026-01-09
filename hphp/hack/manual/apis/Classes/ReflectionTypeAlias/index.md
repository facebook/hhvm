---
title: ReflectionTypeAlias
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The ReflectionTypeAlias class reports information about a type
alias




## Interface Synopsis




``` Hack
class ReflectionTypeAlias implements Reflector {...}
```




### Public Methods




+ [` ->__construct(string $name) `](/docs/apis/Classes/ReflectionTypeAlias/__construct/)\
  Constructs a new ReflectionTypeAlias

+ [` ->__toString(): string `](/docs/apis/Classes/ReflectionTypeAlias/__toString/)

+ [` ->getAssignedTypeText(): string `](/docs/apis/Classes/ReflectionTypeAlias/getAssignedTypeText/)\
  Get the assigned type as a string

+ [` ->getAttribute(string $name): ?varray<mixed> `](/docs/apis/Classes/ReflectionTypeAlias/getAttribute/)

+ [` ->getAttributeClass<T as HH\TypeAliasAttribute>(classname<T> $c): ?T `](/docs/apis/Classes/ReflectionTypeAlias/getAttributeClass/)

+ [` ->getAttributes(): darray<string, varray<mixed>> `](/docs/apis/Classes/ReflectionTypeAlias/getAttributes/)

+ [` ->getAttributesNamespaced(): darray<arraykey, varray<mixed>> `](/docs/apis/Classes/ReflectionTypeAlias/getAttributesNamespaced/)\
  Gets all attributes

+ [` ->getFile(): ReflectionFile `](/docs/apis/Classes/ReflectionTypeAlias/getFile/)\
  Gets the declaring file for the reflected type alias

+ [` ->getFileName(): string `](/docs/apis/Classes/ReflectionTypeAlias/getFileName/)\
  Get the name of the file in which the type alias was defined

+ [` ->getName(): string `](/docs/apis/Classes/ReflectionTypeAlias/getName/)\
  Get the name of the type alias

+ [` ->getResolvedTypeStructure(): darray `](/docs/apis/Classes/ReflectionTypeAlias/getResolvedTypeStructure/)\
  Get the TypeStructure with type information resolved

+ [` ->getTypeStructure(): darray `](/docs/apis/Classes/ReflectionTypeAlias/getTypeStructure/)\
  Get the TypeStructure that contains the full type information of
  the assigned type

+ [` ->hasAttribute(string $name): bool `](/docs/apis/Classes/ReflectionTypeAlias/hasAttribute/)

<!-- HHAPIDOC -->
