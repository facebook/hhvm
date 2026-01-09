---
title: ReflectionMethod
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

( excerpt from http://php.net/manual/en/class.reflectionmethod.php )




The ReflectionMethod class reports information about a method.




## Interface Synopsis




``` Hack
class ReflectionMethod extends ReflectionFunctionAbstract implements Reflector {...}
```




### Public Methods




+ [` ::export(string $class, string $name, bool $return = false): ?string `](/docs/apis/Classes/ReflectionMethod/export/)\
  ( excerpt from http://php.net/manual/en/reflectionmethod.export.php )

+ [` ->__construct(...$class) `](/docs/apis/Classes/ReflectionMethod/__construct/)\
  ( excerpt from http://php.net/manual/en/reflectionmethod.construct.php )

+ [` ->__debugInfo() `](/docs/apis/Classes/ReflectionMethod/__debugInfo/)

+ [` ->__toString(): string `](/docs/apis/Classes/ReflectionMethod/__toString/)\
  ( excerpt from http://php.net/manual/en/reflectionmethod.tostring.php )

+ [` ->getAttributeClass<T as HH\MethodAttribute>(classname<T> $c): ?T `](/docs/apis/Classes/ReflectionMethod/getAttributeClass/)

+ [` ->getClosure($object = NULL): ?Closure `](/docs/apis/Classes/ReflectionMethod/getClosure/)\
  ( excerpt from http://php.net/manual/en/reflectionmethod.getclosure.php
  )

+ [` ->getDeclaringClass() `](/docs/apis/Classes/ReflectionMethod/getDeclaringClass/)\
  ( excerpt from
  http://php.net/manual/en/reflectionmethod.getdeclaringclass.php )

+ [` ->getModifiers(): int `](/docs/apis/Classes/ReflectionMethod/getModifiers/)\
  ( excerpt from
  http://php.net/manual/en/reflectionmethod.getmodifiers.php )

+ [` ->getOriginalClassname(): string `](/docs/apis/Classes/ReflectionMethod/getOriginalClassname/)

+ [` ->getPrototype(): ReflectionMethod `](/docs/apis/Classes/ReflectionMethod/getPrototype/)\
  ( excerpt from
  http://php.net/manual/en/reflectionmethod.getprototype.php )

+ [` ->invoke($obj, ...$args): mixed `](/docs/apis/Classes/ReflectionMethod/invoke/)\
  ( excerpt from http://php.net/manual/en/reflectionmethod.invoke.php )

+ [` ->invokeArgs($obj, varray $args): mixed `](/docs/apis/Classes/ReflectionMethod/invokeArgs/)\
  ( excerpt from http://php.net/manual/en/reflectionmethod.invokeargs.php
  )

+ [` ->isAbstract(): bool `](/docs/apis/Classes/ReflectionMethod/isAbstract/)\
  ( excerpt from http://php.net/manual/en/reflectionmethod.isabstract.php
  )

+ [` ->isConstructor(): bool `](/docs/apis/Classes/ReflectionMethod/isConstructor/)\
  ( excerpt from
  http://php.net/manual/en/reflectionmethod.isconstructor.php )

+ [` ->isFinal(): bool `](/docs/apis/Classes/ReflectionMethod/isFinal/)\
  ( excerpt from http://php.net/manual/en/reflectionmethod.isfinal.php )

+ [` ->isPrivate(): bool `](/docs/apis/Classes/ReflectionMethod/isPrivate/)\
  ( excerpt from http://php.net/manual/en/reflectionmethod.isprivate.php )

+ [` ->isProtected(): bool `](/docs/apis/Classes/ReflectionMethod/isProtected/)\
  ( excerpt from http://php.net/manual/en/reflectionmethod.isprotected.php
  )

+ [` ->isPublic(): bool `](/docs/apis/Classes/ReflectionMethod/isPublic/)\
  ( excerpt from http://php.net/manual/en/reflectionmethod.ispublic.php )

+ [` ->isReadonly(): bool `](/docs/apis/Classes/ReflectionMethod/isReadonly/)

+ [` ->isStatic(): bool `](/docs/apis/Classes/ReflectionMethod/isStatic/)\
  ( excerpt from http://php.net/manual/en/reflectionmethod.isstatic.php )

+ [` ->setAccessible(bool $accessible): void `](/docs/apis/Classes/ReflectionMethod/setAccessible/)\
  ( excerpt from
  http://php.net/manual/en/reflectionmethod.setaccessible.php )

<!-- HHAPIDOC -->
