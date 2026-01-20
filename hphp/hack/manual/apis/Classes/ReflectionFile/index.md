---
title: ReflectionFile
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

The ReflectionFile class reports information about a file




## Interface Synopsis




``` Hack
final class ReflectionFile implements Reflector {...}
```




### Public Methods




+ [` ->__construct(string $name) `](/apis/Classes/ReflectionFile/__construct/)\
  Constructs a new ReflectionFile

+ [` ->__toString(): string `](/apis/Classes/ReflectionFile/__toString/)

+ [` ->getAttribute(string $name): ?varray<mixed> `](/apis/Classes/ReflectionFile/getAttribute/)

+ [` ->getAttributeClass<T as HH\FileAttribute>(classname<T> $c): ?T `](/apis/Classes/ReflectionFile/getAttributeClass/)

+ [` ->getAttributes(): darray<string, varray<mixed>> `](/apis/Classes/ReflectionFile/getAttributes/)

+ [` ->getAttributesNamespaced(): darray<arraykey, varray<mixed>> `](/apis/Classes/ReflectionFile/getAttributesNamespaced/)\
  Gets all attributes

+ [` ->getName(): string `](/apis/Classes/ReflectionFile/getName/)\
  Get the name of the file

+ [` ->hasAttribute(string $name): bool `](/apis/Classes/ReflectionFile/hasAttribute/)

<!-- HHAPIDOC -->
