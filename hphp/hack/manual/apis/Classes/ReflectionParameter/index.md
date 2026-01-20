---
title: ReflectionParameter
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

( excerpt from http://php.net/manual/en/class.reflectionparameter.php )




The ReflectionParameter class retrieves information about function's or
method's parameters.




To introspect function parameters, first create an instance of the
ReflectionFunction or ReflectionMethod classes and then use their
ReflectionFunctionAbstract::getParameters() method to retrieve an array
of parameters.




## Interface Synopsis




``` Hack
class ReflectionParameter implements Reflector {...}
```




### Public Methods




+ [` ::export($function, $parameter, $return = false) `](/apis/Classes/ReflectionParameter/export/)\
  ( excerpt from http://php.net/manual/en/reflectionparameter.export.php )
+ [` ->__construct($function, $parameter, $info = NULL) `](/apis/Classes/ReflectionParameter/__construct/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.construct.php )
+ [` ->__toString() `](/apis/Classes/ReflectionParameter/__toString/)
+ [` ->allowsNull() `](/apis/Classes/ReflectionParameter/allowsNull/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.allowsnull.php )
+ [` ->canBePassedByValue() `](/apis/Classes/ReflectionParameter/canBePassedByValue/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.canbepassedbyvalue.php )
+ [` ->getAttribute(string $name): ?varray<mixed> `](/apis/Classes/ReflectionParameter/getAttribute/)
+ [` ->getAttributeClass<T as HH\ParameterAttribute>(classname<T> $c): ?T `](/apis/Classes/ReflectionParameter/getAttributeClass/)
+ [` ->getAttributes(): darray<string, varray<mixed>> `](/apis/Classes/ReflectionParameter/getAttributes/)
+ [` ->getAttributesNamespaced() `](/apis/Classes/ReflectionParameter/getAttributesNamespaced/)
+ [` ->getClass() `](/apis/Classes/ReflectionParameter/getClass/)\
  ( excerpt from http://php.net/manual/en/reflectionparameter.getclass.php
  )
+ [` ->getDeclaringClass() `](/apis/Classes/ReflectionParameter/getDeclaringClass/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.getdeclaringclass.php )
+ [` ->getDeclaringFunction() `](/apis/Classes/ReflectionParameter/getDeclaringFunction/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.getdeclaringfunction.php )
+ [` ->getDefaultValue() `](/apis/Classes/ReflectionParameter/getDefaultValue/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.getdefaultvalue.php )
+ [` ->getDefaultValueConstantName() `](/apis/Classes/ReflectionParameter/getDefaultValueConstantName/)\
  ( excerpt from
  php.net/manual/en/reflectionparameter.getdefaultvalueconstantname.php
  )
+ [` ->getDefaultValueText() `](/apis/Classes/ReflectionParameter/getDefaultValueText/)\
  This is an HHVM only function that gets the raw text associated with
  a default parameter
+ [` ->getName() `](/apis/Classes/ReflectionParameter/getName/)\
  ( excerpt from http://php.net/manual/en/reflectionparameter.getname.php
  )
+ [` ->getPosition() `](/apis/Classes/ReflectionParameter/getPosition/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.getposition.php )
+ [` ->getType(): ?ReflectionType `](/apis/Classes/ReflectionParameter/getType/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.gettype.php )
+ [` ->getTypeText(): string `](/apis/Classes/ReflectionParameter/getTypeText/)
+ [` ->getTypehintText() `](/apis/Classes/ReflectionParameter/getTypehintText/)
+ [` ->hasAttribute(string $name): bool `](/apis/Classes/ReflectionParameter/hasAttribute/)
+ [` ->hasType(): bool `](/apis/Classes/ReflectionParameter/hasType/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.hastype.php )
+ [` ->isArray() `](/apis/Classes/ReflectionParameter/isArray/)\
  ( excerpt from http://php.net/manual/en/reflectionparameter.isarray.php
  )
+ [` ->isCallable() `](/apis/Classes/ReflectionParameter/isCallable/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.iscallable.php )
+ [` ->isDefaultValueAvailable() `](/apis/Classes/ReflectionParameter/isDefaultValueAvailable/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.isdefaultvalueavailable.php
  )
+ [` ->isDefaultValueConstant(): ?bool `](/apis/Classes/ReflectionParameter/isDefaultValueConstant/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.isdefaultvalueconstant.php )
+ [` ->isInOut(): bool `](/apis/Classes/ReflectionParameter/isInOut/)\
  Checks if this is an inout parameter
+ [` ->isOptional(): bool `](/apis/Classes/ReflectionParameter/isOptional/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.isoptional.php )
+ [` ->isPassedByReference() `](/apis/Classes/ReflectionParameter/isPassedByReference/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.ispassedbyreference.php )
+ [` ->isReadonly(): bool `](/apis/Classes/ReflectionParameter/isReadonly/)\
  Checks if this is a readonly parameter
+ [` ->isVariadic(): bool `](/apis/Classes/ReflectionParameter/isVariadic/)\
  Checks if the parameter is variadic
+ [` ->toString() `](/apis/Classes/ReflectionParameter/__toString/)\
  ( excerpt from http://php.net/manual/en/reflectionparameter.tostring.php
  )
<!-- HHAPIDOC -->
