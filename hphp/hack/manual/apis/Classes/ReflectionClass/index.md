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




+ [` ::export(mixed $argument, bool $return = false): ?string `](/apis/Classes/ReflectionClass/export/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.export.php )

+ [` ->__clone() `](/apis/Classes/ReflectionClass/__clone/)

+ [` ->__construct(mixed $name_or_obj) `](/apis/Classes/ReflectionClass/__construct/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.construct.php )

+ [` ->__toString(): string `](/apis/Classes/ReflectionClass/__toString/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.tostring.php )

+ [` ->getAbstractConstantNames(): darray<string> `](/apis/Classes/ReflectionClass/getAbstractConstantNames/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getabstractconstantnames.php
  )

+ [` ->getAttribute(string $name): ?varray<mixed> `](/apis/Classes/ReflectionClass/getAttribute/)

+ [` ->getAttributeClass<T as HH\ClassLikeAttribute>(classname<T> $c): ?T `](/apis/Classes/ReflectionClass/getAttributeClass/)

+ [` ->getAttributes(): darray<string, varray<mixed>> `](/apis/Classes/ReflectionClass/getAttributes/)

+ [` ->getAttributesNamespaced(): darray<string, varray<mixed>> `](/apis/Classes/ReflectionClass/getAttributesNamespaced/)

+ [` ->getAttributesRecursiveNamespaced(): darray<string, varray<mixed>> `](/apis/Classes/ReflectionClass/getAttributesRecursiveNamespaced/)

+ [` ->getConstant(string $name): mixed `](/apis/Classes/ReflectionClass/getConstant/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.getconstant.php
  )

+ [` ->getConstants(): darray<string, mixed> `](/apis/Classes/ReflectionClass/getConstants/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.getconstants.php
  )

+ [` ->getConstructor(): ?ReflectionMethod `](/apis/Classes/ReflectionClass/getConstructor/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getconstructor.php )

+ [` ->getDefaultProperties(): darray<string, mixed> `](/apis/Classes/ReflectionClass/getDefaultProperties/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getdefaultproperties.php )

+ [` ->getDocComment(): mixed `](/apis/Classes/ReflectionClass/getDocComment/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getdoccomment.php )

+ [` ->getEndLine(): int `](/apis/Classes/ReflectionClass/getEndLine/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.getendline.php )

+ [` ->getEnumUnderlyingType(): string `](/apis/Classes/ReflectionClass/getEnumUnderlyingType/)\
  Returns the underlying type of this ReflectionClass, given that it
  represents an enum

+ [` ->getExtension(): ?ReflectionExtension `](/apis/Classes/ReflectionClass/getExtension/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.getextension.php
  )

+ [` ->getExtensionName(): string `](/apis/Classes/ReflectionClass/getExtensionName/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getextensionname.php )

+ [` ->getFile(): ReflectionFile `](/apis/Classes/ReflectionClass/getFile/)\
  Gets the declaring file for the reflected class

+ [` ->getFileName(): mixed `](/apis/Classes/ReflectionClass/getFileName/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.getfilename.php
  )

+ [` ->getInterfaceNames(): varray<string> `](/apis/Classes/ReflectionClass/getInterfaceNames/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getinterfacenames.php )

+ [` ->getInterfaces(): darray<string, ReflectionClass> `](/apis/Classes/ReflectionClass/getInterfaces/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getinterfaces.php )

+ [` ->getMethod(string $name): ReflectionMethod `](/apis/Classes/ReflectionClass/getMethod/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.getmethod.php )

+ [` ->getMethods(?int $filter = NULL): varray<ReflectionMethod> `](/apis/Classes/ReflectionClass/getMethods/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.getmethods.php )

+ [` ->getModifiers(): int `](/apis/Classes/ReflectionClass/getModifiers/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.getmodifiers.php
  )

+ [` ->getModule(): ?string `](/apis/Classes/ReflectionClass/getModule/)

+ [` ->getName(): string `](/apis/Classes/ReflectionClass/getName/)

+ [` ->getNamespaceName(): string `](/apis/Classes/ReflectionClass/getNamespaceName/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.functionabstract.php )

+ [` ->getParentClass(): mixed `](/apis/Classes/ReflectionClass/getParentClass/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getparentclass.php )

+ [` ->getProperties(int $filter = 65535): varray<ReflectionProperty> `](/apis/Classes/ReflectionClass/getProperties/)\
  ( excerpt* http://php.net/manual/en/reflectionclass.getproperties.php )

+ [` ->getProperty(string $name): ReflectionProperty `](/apis/Classes/ReflectionClass/getProperty/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.getproperty.php
  )

+ [` ->getReifiedTypeParamInfo(): varray<shape('is_reified' => bool, 'is_soft' => bool, 'is_warn' => bool)> `](/apis/Classes/ReflectionClass/getReifiedTypeParamInfo/)

+ [` ->getRequirementNames(): varray<string> `](/apis/Classes/ReflectionClass/getRequirementNames/)\
  Gets the list of implemented interfaces/inherited classes needed to
  implement an interface / use a trait

+ [` ->getRequirements(): darray<string, ReflectionClass> `](/apis/Classes/ReflectionClass/getRequirements/)\
  Gets ReflectionClass-es for the requirements of this class

+ [` ->getShortName(): string `](/apis/Classes/ReflectionClass/getShortName/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getshortname.php )

+ [` ->getStartLine(): int `](/apis/Classes/ReflectionClass/getStartLine/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.getstartline.php
  )

+ [` ->getStaticProperties(): darray<string, mixed> `](/apis/Classes/ReflectionClass/getStaticProperties/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getstaticproperties.php )

+ [` ->getStaticPropertyValue(string $name, mixed ...$def_value = NULL): mixed `](/apis/Classes/ReflectionClass/getStaticPropertyValue/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.getstaticpropertyvalue.php )

+ [` ->getTraitAliases(): darray<string, string> `](/apis/Classes/ReflectionClass/getTraitAliases/)

+ [` ->getTraitNames(): varray<string> `](/apis/Classes/ReflectionClass/getTraitNames/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.gettraitnames.php )

+ [` ->getTraits(): darray<string, ReflectionClass> `](/apis/Classes/ReflectionClass/getTraits/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.gettraits.php )

+ [` ->getTypeConstant(string $name): ReflectionTypeConstant `](/apis/Classes/ReflectionClass/getTypeConstant/)

+ [` ->getTypeConstants(): varray<ReflectionTypeConstant> `](/apis/Classes/ReflectionClass/getTypeConstants/)

+ [` ->hasAttribute(string $name): bool `](/apis/Classes/ReflectionClass/hasAttribute/)

+ [` ->hasConstant(string $name): bool `](/apis/Classes/ReflectionClass/hasConstant/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.hasconstant.php
  )

+ [` ->hasMethod(string $name): bool `](/apis/Classes/ReflectionClass/hasMethod/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.hasmethod.php )

+ [` ->hasProperty(string $name): bool `](/apis/Classes/ReflectionClass/hasProperty/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.hasproperty.php
  )

+ [` ->hasTypeConstant(string $name): bool `](/apis/Classes/ReflectionClass/hasTypeConstant/)

+ [` ->implementsInterface(string $interface): bool `](/apis/Classes/ReflectionClass/implementsInterface/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.implementsinterface.php )

+ [` ->inNamespace(): bool `](/apis/Classes/ReflectionClass/inNamespace/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.innamespace.php
  )

+ [` ->isAbstract(): bool `](/apis/Classes/ReflectionClass/isAbstract/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.isabstract.php )

+ [` ->isCloneable(): bool `](/apis/Classes/ReflectionClass/isCloneable/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.iscloneable.php )

+ [` ->isEnum(): bool `](/apis/Classes/ReflectionClass/isEnum/)\
  Returns whether this ReflectionClass represents an enum

+ [` ->isFinal(): bool `](/apis/Classes/ReflectionClass/isFinal/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.isfinal.php )

+ [` ->isHack(): bool `](/apis/Classes/ReflectionClass/isHack/)

+ [` ->isInstance(mixed $obj): bool `](/apis/Classes/ReflectionClass/isInstance/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.isinstance.php )

+ [` ->isInstantiable(): bool `](/apis/Classes/ReflectionClass/isInstantiable/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.isinstantiable.php )

+ [` ->isInterface(): bool `](/apis/Classes/ReflectionClass/isInterface/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.isinterface.php
  )

+ [` ->isInternal(): bool `](/apis/Classes/ReflectionClass/isInternal/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.isinternal.php )

+ [` ->isInternalToModule(): bool `](/apis/Classes/ReflectionClass/isInternalToModule/)\
  Checks if a class is internal

+ [` ->isIterateable(): bool `](/apis/Classes/ReflectionClass/isIterateable/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.isiterateable.php )

+ [` ->isSubclassOf(mixed $class): bool `](/apis/Classes/ReflectionClass/isSubclassOf/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.issubclassof.php
  )

+ [` ->isTrait(): bool `](/apis/Classes/ReflectionClass/isTrait/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.istrait.php )

+ [` ->isUserDefined(): bool `](/apis/Classes/ReflectionClass/isUserDefined/)

+ [` ->newInstance(...$args) `](/apis/Classes/ReflectionClass/newInstance/)\
  ( excerpt from http://php.net/manual/en/reflectionclass.newinstance.php
  )

+ [` ->newInstanceArgs(Traversable<mixed> $args = vec [ ]) `](/apis/Classes/ReflectionClass/newInstanceArgs/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.newinstanceargs.php )

+ [` ->newInstanceWithoutConstructor() `](/apis/Classes/ReflectionClass/newInstanceWithoutConstructor/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.newinstancewithoutconstructor.php
  )

+ [` ->setStaticPropertyValue(string $name, mixed $value): void `](/apis/Classes/ReflectionClass/setStaticPropertyValue/)\
  ( excerpt from
  http://php.net/manual/en/reflectionclass.setstaticpropertyvalue.php )

<!-- HHAPIDOC -->
