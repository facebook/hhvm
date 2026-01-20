---
title: ReflectionModule
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The ReflectionModule class reports information about a module




## Interface Synopsis




``` Hack
final class ReflectionModule implements Reflector {...}
```




### Public Methods




+ [` ->__construct(string $name) `](/apis/Classes/ReflectionModule/__construct/)\
  Constructs a new ReflectionModule

+ [` ->__toString(): string `](/apis/Classes/ReflectionModule/__toString/)

+ [` ->getAttribute(string $name): ?varray<mixed> `](/apis/Classes/ReflectionModule/getAttribute/)

+ [` ->getAttributeClass<T as HH\FileAttribute>(classname<T> $c): ?T `](/apis/Classes/ReflectionModule/getAttributeClass/)

+ [` ->getAttributes(): darray<string, varray<mixed>> `](/apis/Classes/ReflectionModule/getAttributes/)

+ [` ->getAttributesNamespaced(): darray<arraykey, varray<mixed>> `](/apis/Classes/ReflectionModule/getAttributesNamespaced/)\
  Gets all attributes

+ [` ->getName(): string `](/apis/Classes/ReflectionModule/getName/)\
  Get the name of the file

+ [` ->hasAttribute(string $name): bool `](/apis/Classes/ReflectionModule/hasAttribute/)

<!-- HHAPIDOC -->
