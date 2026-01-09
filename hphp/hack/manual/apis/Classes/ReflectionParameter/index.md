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




+ [` ::export($function, $parameter, $return = false) `](/docs/apis/Classes/ReflectionParameter/export/)\
  ( excerpt from http://php.net/manual/en/reflectionparameter.export.php )
+ [` ->__construct($function, $parameter, $info = NULL) `](/docs/apis/Classes/ReflectionParameter/__construct/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.construct.php )
+ [` ->__toString() `](/docs/apis/Classes/ReflectionParameter/__toString/)
+ [` ->allowsNull() `](/docs/apis/Classes/ReflectionParameter/allowsNull/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.allowsnull.php )
+ [` ->canBePassedByValue() `](/docs/apis/Classes/ReflectionParameter/canBePassedByValue/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.canbepassedbyvalue.php )
+ [` ->getAttribute(string $name): ?varray<mixed> `](/docs/apis/Classes/ReflectionParameter/getAttribute/)
+ [` ->getAttributeClass<T as HH\ParameterAttribute>(classname<T> $c): ?T `](/docs/apis/Classes/ReflectionParameter/getAttributeClass/)
+ [` ->getAttributes(): darray<string, varray<mixed>> `](/docs/apis/Classes/ReflectionParameter/getAttributes/)
+ [` ->getAttributesNamespaced() `](/docs/apis/Classes/ReflectionParameter/getAttributesNamespaced/)
+ [` ->getClass() `](/docs/apis/Classes/ReflectionParameter/getClass/)\
  ( excerpt from http://php.net/manual/en/reflectionparameter.getclass.php
  )
+ [` ->getDeclaringClass() `](/docs/apis/Classes/ReflectionParameter/getDeclaringClass/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.getdeclaringclass.php )
+ [` ->getDeclaringFunction() `](/docs/apis/Classes/ReflectionParameter/getDeclaringFunction/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.getdeclaringfunction.php )
+ [` ->getDefaultValue() `](/docs/apis/Classes/ReflectionParameter/getDefaultValue/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.getdefaultvalue.php )
+ [` ->getDefaultValueConstantName() `](/docs/apis/Classes/ReflectionParameter/getDefaultValueConstantName/)\
  ( excerpt from
  php.net/manual/en/reflectionparameter.getdefaultvalueconstantname.php
  )
+ [` ->getDefaultValueText() `](/docs/apis/Classes/ReflectionParameter/getDefaultValueText/)\
  This is an HHVM only function that gets the raw text associated with
  a default parameter
+ [` ->getName() `](/docs/apis/Classes/ReflectionParameter/getName/)\
  ( excerpt from http://php.net/manual/en/reflectionparameter.getname.php
  )
+ [` ->getPosition() `](/docs/apis/Classes/ReflectionParameter/getPosition/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.getposition.php )
+ [` ->getType(): ?ReflectionType `](/docs/apis/Classes/ReflectionParameter/getType/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.gettype.php )
+ [` ->getTypeText(): string `](/docs/apis/Classes/ReflectionParameter/getTypeText/)
+ [` ->getTypehintText() `](/docs/apis/Classes/ReflectionParameter/getTypehintText/)
+ [` ->hasAttribute(string $name): bool `](/docs/apis/Classes/ReflectionParameter/hasAttribute/)
+ [` ->hasType(): bool `](/docs/apis/Classes/ReflectionParameter/hasType/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.hastype.php )
+ [` ->isArray() `](/docs/apis/Classes/ReflectionParameter/isArray/)\
  ( excerpt from http://php.net/manual/en/reflectionparameter.isarray.php
  )
+ [` ->isCallable() `](/docs/apis/Classes/ReflectionParameter/isCallable/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.iscallable.php )
+ [` ->isDefaultValueAvailable() `](/docs/apis/Classes/ReflectionParameter/isDefaultValueAvailable/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.isdefaultvalueavailable.php
  )
+ [` ->isDefaultValueConstant(): ?bool `](/docs/apis/Classes/ReflectionParameter/isDefaultValueConstant/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.isdefaultvalueconstant.php )
+ [` ->isInOut(): bool `](/docs/apis/Classes/ReflectionParameter/isInOut/)\
  Checks if this is an inout parameter
+ [` ->isOptional(): bool `](/docs/apis/Classes/ReflectionParameter/isOptional/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.isoptional.php )
+ [` ->isPassedByReference() `](/docs/apis/Classes/ReflectionParameter/isPassedByReference/)\
  ( excerpt from
  http://php.net/manual/en/reflectionparameter.ispassedbyreference.php )
+ [` ->isReadonly(): bool `](/docs/apis/Classes/ReflectionParameter/isReadonly/)\
  Checks if this is a readonly parameter
+ [` ->isVariadic(): bool `](/docs/apis/Classes/ReflectionParameter/isVariadic/)\
  Checks if the parameter is variadic
+ [` ->toString() `](/docs/apis/Classes/ReflectionParameter/__toString/)\
  ( excerpt from http://php.net/manual/en/reflectionparameter.tostring.php
  )
<!-- HHAPIDOC -->
