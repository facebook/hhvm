---
title: ReflectionFunction
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

( excerpt from http://php.net/manual/en/class.reflectionfunction.php )




The ReflectionFunction class reports information about a function.




## Interface Synopsis




``` Hack
class ReflectionFunction extends ReflectionFunctionAbstract implements Reflector {...}
```




### Public Methods




+ [` ::export($name, $return = false) `](/apis/Classes/ReflectionFunction/export/)\
  ( excerpt from http://php.net/manual/en/reflectionfunction.export.php )

+ [` ->__construct($name_or_closure) `](/apis/Classes/ReflectionFunction/__construct/)\
  ( excerpt from http://php.net/manual/en/reflectionfunction.construct.php
  )

+ [` ->__toString(): string `](/apis/Classes/ReflectionFunction/__toString/)\
  ( excerpt from http://php.net/manual/en/reflectionfunction.tostring.php )

+ [` ->getAttributeClass<T as HH\FunctionAttribute>(classname<T> $c): ?T `](/apis/Classes/ReflectionFunction/getAttributeClass/)

+ [` ->getClosure() `](/apis/Classes/ReflectionFunction/getClosure/)

+ [` ->getClosureScopeClass(): ?ReflectionClass `](/apis/Classes/ReflectionFunction/getClosureScopeClass/)

+ [` ->getClosureThis(): mixed `](/apis/Classes/ReflectionFunction/getClosureThis/)\
  Returns this pointer bound to closure

+ [` ->getName(): string `](/apis/Classes/ReflectionFunction/getName/)\
  (excerpt from
  http://php.net/manual/en/reflectionfunctionabstract.getname.php )

+ [` ->invoke(...$args) `](/apis/Classes/ReflectionFunction/invoke/)\
  ( excerpt from http://php.net/manual/en/reflectionfunction.invoke.php )

+ [` ->invokeArgs(varray $args) `](/apis/Classes/ReflectionFunction/invokeArgs/)\
  ( excerpt from
  http://php.net/manual/en/reflectionfunction.invokeargs.php )

+ [` ->isClosure(): bool `](/apis/Classes/ReflectionFunction/isClosure/)

+ [` ->isDisabled(): bool `](/apis/Classes/ReflectionFunction/isDisabled/)\
  ( excerpt from
  http://php.net/manual/en/reflectionfunction.isdisabled.php )

<!-- HHAPIDOC -->
