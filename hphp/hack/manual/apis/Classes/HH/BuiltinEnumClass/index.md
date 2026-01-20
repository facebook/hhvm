---
title: BuiltinEnumClass
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

BuiltinEnumClass contains the utility methods provided by enum classes




Under the hood, an enum class Foo : Bar will extend
`BuiltinEnumClass<HH\\MemberOf<this, Bar>>`.




HHVM provides a native implementation for this class. The PHP class
definition below is not actually used at run time; it is simply
provided for the typechecker and for developer reference.




## Interface Synopsis




``` Hack
namespace HH;

abstract class BuiltinEnumClass extends BuiltinAbstractEnumClass {...}
```




### Public Methods




+ [` ::getValues(): darray<string, T> `](/apis/Classes/HH/BuiltinEnumClass/getValues/)\
  Get the values of the public consts defined on this class,
  indexed by the string name of those consts
+ [` ::valueOf<TEnum super this, TType>(EnumClass\Label<TEnum, TType> $label): MemberOf<TEnum, TType> `](/apis/Classes/HH/BuiltinEnumClass/valueOf/)







### Public Methods ([` HH\BuiltinAbstractEnumClass `](/apis/Classes/HH/BuiltinAbstractEnumClass/))




* [` ::nameOf<TType>(EnumClass\Label<this, TType> $label): string `](/apis/Classes/HH/BuiltinAbstractEnumClass/nameOf/)
<!-- HHAPIDOC -->
