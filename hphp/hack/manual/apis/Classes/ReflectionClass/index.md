---
title: ReflectionClass
---


:::info[Note]
This is a point-in-time snapshot of the API documentation from January 2026.
Going forward, we will not be maintaining a public copy of these references,
and recommend users to refer to the built-in signature helpers available in
the Hack LSP instead for complete and up-to-date information.
:::

( excerpt from http://php.net/manual/en/class.reflectionclass.php )




The ReflectionClass class reports information about a class.




## Interface Synopsis




``` Hack
class ReflectionClass implements Reflector {...}
```




### Public Methods




+ [` ::export(mixed $argument, bool $return = false): ?string `](/docs/apis/Classes/ReflectionClass/export/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.export.php )

+ [` ->__clone() `](/docs/apis/Classes/ReflectionClass/__clone/)

+ [` ->__construct(mixed $name_or_obj) `](/docs/apis/Classes/ReflectionClass/__construct/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.construct.php )

+ [` ->__toString(): string `](/docs/apis/Classes/ReflectionClass/__toString/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.tostring.php )

+ [` ->getAbstractConstantNames(): darray<string> `](/docs/apis/Classes/ReflectionClass/getAbstractConstantNames/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getabstractconstantnames.php
  )

+ [` ->getAttribute(string $name): ?varray<mixed> `](/docs/apis/Classes/ReflectionClass/getAttribute/)

+ [` ->getAttributeClass<T as HH\ClassLikeAttribute>(classname<T> $c): ?T `](/docs/apis/Classes/ReflectionClass/getAttributeClass/)

+ [` ->getAttributes(): darray<string, varray<mixed>> `](/docs/apis/Classes/ReflectionClass/getAttributes/)

+ [` ->getAttributesNamespaced(): darray<string, varray<mixed>> `](/docs/apis/Classes/ReflectionClass/getAttributesNamespaced/)

+ [` ->getAttributesRecursiveNamespaced(): darray<string, varray<mixed>> `](/docs/apis/Classes/ReflectionClass/getAttributesRecursiveNamespaced/)

+ [` ->getConstant(string $name): mixed `](/docs/apis/Classes/ReflectionClass/getConstant/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.getconstant.php
  )

+ [` ->getConstants(): darray<string, mixed> `](/docs/apis/Classes/ReflectionClass/getConstants/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.getconstants.php
  )

+ [` ->getConstructor(): ?ReflectionMethod `](/docs/apis/Classes/ReflectionClass/getConstructor/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getconstructor.php )

+ [` ->getDefaultProperties(): darray<string, mixed> `](/docs/apis/Classes/ReflectionClass/getDefaultProperties/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getdefaultproperties.php )

+ [` ->getDocComment(): mixed `](/docs/apis/Classes/ReflectionClass/getDocComment/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getdoccomment.php )

+ [` ->getEndLine(): int `](/docs/apis/Classes/ReflectionClass/getEndLine/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.getendline.php )

+ [` ->getEnumUnderlyingType(): string `](/docs/apis/Classes/ReflectionClass/getEnumUnderlyingType/)\
  Returns the underlying type of this ReflectionClass, given that it
  represents an enum

+ [` ->getExtension(): ?ReflectionExtension `](/docs/apis/Classes/ReflectionClass/getExtension/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.getextension.php
  )

+ [` ->getExtensionName(): string `](/docs/apis/Classes/ReflectionClass/getExtensionName/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getextensionname.php )

+ [` ->getFile(): ReflectionFile `](/docs/apis/Classes/ReflectionClass/getFile/)\
  Gets the declaring file for the reflected class

+ [` ->getFileName(): mixed `](/docs/apis/Classes/ReflectionClass/getFileName/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.getfilename.php
  )

+ [` ->getInterfaceNames(): varray<string> `](/docs/apis/Classes/ReflectionClass/getInterfaceNames/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getinterfacenames.php )

+ [` ->getInterfaces(): darray<string, ReflectionClass> `](/docs/apis/Classes/ReflectionClass/getInterfaces/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getinterfaces.php )

+ [` ->getMethod(string $name): ReflectionMethod `](/docs/apis/Classes/ReflectionClass/getMethod/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.getmethod.php )

+ [` ->getMethods(?int $filter = NULL): varray<ReflectionMethod> `](/docs/apis/Classes/ReflectionClass/getMethods/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.getmethods.php )

+ [` ->getModifiers(): int `](/docs/apis/Classes/ReflectionClass/getModifiers/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.getmodifiers.php
  )

+ [` ->getModule(): ?string `](/docs/apis/Classes/ReflectionClass/getModule/)

+ [` ->getName(): string `](/docs/apis/Classes/ReflectionClass/getName/)

+ [` ->getNamespaceName(): string `](/docs/apis/Classes/ReflectionClass/getNamespaceName/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.functionabstract.php )

+ [` ->getParentClass(): mixed `](/docs/apis/Classes/ReflectionClass/getParentClass/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getparentclass.php )

+ [` ->getProperties(int $filter = 65535): varray<ReflectionProperty> `](/docs/apis/Classes/ReflectionClass/getProperties/)\
  ( excerpt* http://php.net/manual/en/reflectionclass.getproperties.php )

+ [` ->getProperty(string $name): ReflectionProperty `](/docs/apis/Classes/ReflectionClass/getProperty/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.getproperty.php
  )

+ [` ->getReifiedTypeParamInfo(): varray<shape('is_reified' => bool, 'is_soft' => bool, 'is_warn' => bool)> `](/docs/apis/Classes/ReflectionClass/getReifiedTypeParamInfo/)

+ [` ->getRequirementNames(): varray<string> `](/docs/apis/Classes/ReflectionClass/getRequirementNames/)\
  Gets the list of implemented interfaces/inherited classes needed to
  implement an interface / use a trait

+ [` ->getRequirements(): darray<string, ReflectionClass> `](/docs/apis/Classes/ReflectionClass/getRequirements/)\
  Gets ReflectionClass-es for the requirements of this class

+ [` ->getShortName(): string `](/docs/apis/Classes/ReflectionClass/getShortName/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getshortname.php )

+ [` ->getStartLine(): int `](/docs/apis/Classes/ReflectionClass/getStartLine/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.getstartline.php
  )

+ [` ->getStaticProperties(): darray<string, mixed> `](/docs/apis/Classes/ReflectionClass/getStaticProperties/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getstaticproperties.php )

+ [` ->getStaticPropertyValue(string $name, mixed ...$def_value = NULL): mixed `](/docs/apis/Classes/ReflectionClass/getStaticPropertyValue/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getstaticpropertyvalue.php )

+ [` ->getTraitAliases(): darray<string, string> `](/docs/apis/Classes/ReflectionClass/getTraitAliases/)

+ [` ->getTraitNames(): varray<string> `](/docs/apis/Classes/ReflectionClass/getTraitNames/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.gettraitnames.php )

+ [` ->getTraits(): darray<string, ReflectionClass> `](/docs/apis/Classes/ReflectionClass/getTraits/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.gettraits.php )

+ [` ->getTypeConstant(string $name): ReflectionTypeConstant `](/docs/apis/Classes/ReflectionClass/getTypeConstant/)

+ [` ->getTypeConstants(): varray<ReflectionTypeConstant> `](/docs/apis/Classes/ReflectionClass/getTypeConstants/)

+ [` ->hasAttribute(string $name): bool `](/docs/apis/Classes/ReflectionClass/hasAttribute/)

+ [` ->hasConstant(string $name): bool `](/docs/apis/Classes/ReflectionClass/hasConstant/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.hasconstant.php
  )

+ [` ->hasMethod(string $name): bool `](/docs/apis/Classes/ReflectionClass/hasMethod/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.hasmethod.php )

+ [` ->hasProperty(string $name): bool `](/docs/apis/Classes/ReflectionClass/hasProperty/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.hasproperty.php
  )

+ [` ->hasTypeConstant(string $name): bool `](/docs/apis/Classes/ReflectionClass/hasTypeConstant/)

+ [` ->implementsInterface(string $interface): bool `](/docs/apis/Classes/ReflectionClass/implementsInterface/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.implementsinterface.php )

+ [` ->inNamespace(): bool `](/docs/apis/Classes/ReflectionClass/inNamespace/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.innamespace.php
  )

+ [` ->isAbstract(): bool `](/docs/apis/Classes/ReflectionClass/isAbstract/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.isabstract.php )

+ [` ->isCloneable(): bool `](/docs/apis/Classes/ReflectionClass/isCloneable/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.iscloneable.php )

+ [` ->isEnum(): bool `](/docs/apis/Classes/ReflectionClass/isEnum/)\
  Returns whether this ReflectionClass represents an enum

+ [` ->isFinal(): bool `](/docs/apis/Classes/ReflectionClass/isFinal/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.isfinal.php )

+ [` ->isHack(): bool `](/docs/apis/Classes/ReflectionClass/isHack/)

+ [` ->isInstance(mixed $obj): bool `](/docs/apis/Classes/ReflectionClass/isInstance/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.isinstance.php )

+ [` ->isInstantiable(): bool `](/docs/apis/Classes/ReflectionClass/isInstantiable/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.isinstantiable.php )

+ [` ->isInterface(): bool `](/docs/apis/Classes/ReflectionClass/isInterface/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.isinterface.php
  )

+ [` ->isInternal(): bool `](/docs/apis/Classes/ReflectionClass/isInternal/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.isinternal.php )

+ [` ->isInternalToModule(): bool `](/docs/apis/Classes/ReflectionClass/isInternalToModule/)\
  Checks if a class is internal

+ [` ->isIterateable(): bool `](/docs/apis/Classes/ReflectionClass/isIterateable/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.isiterateable.php )

+ [` ->isSubclassOf(mixed $class): bool `](/docs/apis/Classes/ReflectionClass/isSubclassOf/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.issubclassof.php
  )

+ [` ->isTrait(): bool `](/docs/apis/Classes/ReflectionClass/isTrait/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.istrait.php )

+ [` ->isUserDefined(): bool `](/docs/apis/Classes/ReflectionClass/isUserDefined/)

+ [` ->newInstance(...$args) `](/docs/apis/Classes/ReflectionClass/newInstance/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.newinstance.php
  )

+ [` ->newInstanceArgs(Traversable<mixed> $args = vec [ ]) `](/docs/apis/Classes/ReflectionClass/newInstanceArgs/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.newinstanceargs.php )

+ [` ->newInstanceWithoutConstructor() `](/docs/apis/Classes/ReflectionClass/newInstanceWithoutConstructor/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.newinstancewithoutconstructor.php
  )

+ [` ->setStaticPropertyValue(string $name, mixed $value): void `](/docs/apis/Classes/ReflectionClass/setStaticPropertyValue/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.setstaticpropertyvalue.php )

<!-- HHAPIDOC -->
