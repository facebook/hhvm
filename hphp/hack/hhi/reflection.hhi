<?hh /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

interface Reflector {
  <<__Pure, __MaybeMutable>>
  public function __toString();
}

class Reflection  {
  <<__Pure>>
  public static function getModifierNames($modifiers);
  public static function export(Reflector $reflector, $return = false);
}

class ReflectionClass implements Reflector {

  const int IS_IMPLICIT_ABSTRACT = 16;
  const int IS_EXPLICIT_ABSTRACT = 32;
  const int IS_FINAL = 64;

  /**
   * This field is read-only
   */
  public string $name;

  <<__Pure>>
  public function __construct(<<__MaybeMutable>> mixed $argument);
  public static function export(mixed $argument, bool $return = false): ?string;
  <<__Pure, __MaybeMutable>>
  public function getConstant(string $name): mixed;
  <<__Pure, __MaybeMutable>>
  public function getConstants(): darray<string, mixed>;
  <<__Pure, __MaybeMutable>>
  public function getAbstractConstantNames(): darray<string, string>;
  <<__Pure, __MaybeMutable>>
  public function getTypeConstant(string $name): ReflectionTypeConstant;
  <<__Pure, __MaybeMutable>>
  public function getTypeConstants(): varray<ReflectionTypeConstant>;
  <<__Pure, __MaybeMutable>>
  public function getConstructor(): ?ReflectionMethod;
  <<__Pure, __MaybeMutable>>
  public function getDefaultProperties(): darray<string, mixed>;
  /**
   * Returns string or false
   */
  <<__Pure, __MaybeMutable>>
  public function getDocComment(): mixed;
  <<__Pure, __MaybeMutable>>
  public function getEndLine(): int;
  <<__Pure, __MaybeMutable>>
  public function getExtension(): ?ReflectionExtension;
  <<__Pure, __MaybeMutable>>
  public function getExtensionName(): string;
  /**
   * Returns string or false
   */
  <<__Pure, __MaybeMutable>>
  public function getFileName(): mixed;
  <<__Pure, __MaybeMutable>>
  public function getFile(): ReflectionFile;
  <<__Pure, __MaybeMutable>>
  public function getInterfaceNames(): varray<string>;
  <<__Pure, __MaybeMutable>>
  public function getInterfaces(): darray<string, ReflectionClass>;
  <<__Pure, __MaybeMutable>>
  final public function getAttributes(): darray<string, varray<mixed>>;
  <<__Pure, __MaybeMutable>>
  final public function hasAttribute(string $name): bool;
  <<__Pure, __MaybeMutable>>
  final public function getAttribute(string $name): ?varray<mixed>;
  <<__Pure, __MaybeMutable>>
  final public function getAttributeClass<T as HH\ClassLikeAttribute>(classname<T> $c): ?T;
  <<__Pure, __MaybeMutable>>
  public function getMethod(string $name): ReflectionMethod;
  <<__Pure, __MaybeMutable>>
  public function getMethods(?int $filter = null): varray<ReflectionMethod>;
  <<__Pure, __MaybeMutable>>
  public function getModifiers(): int;
  <<__Pure, __MaybeMutable>>
  public function getName(): string;
  <<__Pure, __MaybeMutable>>
  public function getNamespaceName(): string;
  /**
   * Returns ReflectionClass or false
   */
  <<__Pure, __MaybeMutable>>
  public function getParentClass(): mixed;
  <<__Pure, __MaybeMutable>>
  public function getProperties(int $filter = 0xFFFF): varray<ReflectionProperty>;
  <<__Pure, __MaybeMutable>>
  public function getProperty(string $name): ReflectionProperty;
  <<__Pure, __MaybeMutable>>
  public function getRequirementNames(): varray<string>;
  <<__Pure, __MaybeMutable>>
  public function getRequirements(): darray<string, ReflectionClass>;
  <<__Pure, __MaybeMutable>>
  public function getShortName(): string;
  <<__Pure, __MaybeMutable>>
  public function getStartLine(): int;
  public function getStaticProperties(): darray<string, mixed>;
  public function getStaticPropertyValue(string $name, mixed $def_value = null): mixed;
  <<__Pure, __MaybeMutable>>
  public function getTraitAliases(): darray<string, string>;
  <<__Pure, __MaybeMutable>>
  public function getTraitNames(): varray<string>;
  <<__Pure, __MaybeMutable>>
  public function getTraits(): darray<string, ReflectionClass>;
  <<__Pure, __MaybeMutable>>
  public function hasConstant(string $name): bool;
  <<__Pure, __MaybeMutable>>
  public function hasMethod(string $name): bool;
  <<__Pure, __MaybeMutable>>
  public function hasProperty(string $name): bool;
  <<__Pure, __MaybeMutable>>
  public function hasTypeConstant(string $name): bool;
  <<__Pure, __MaybeMutable>>
  public function implementsInterface(string $interface): bool;
  <<__Pure, __MaybeMutable>>
  public function inNamespace(): bool;
  <<__Pure, __MaybeMutable>>
  public function isAbstract(): bool;
  <<__Pure, __MaybeMutable>>
  public function isCloneable(): bool;
  <<__Pure, __MaybeMutable>>
  public function isFinal(): bool;
  <<__Pure, __MaybeMutable>>
  public function isInstance(<<__MaybeMutable>> mixed $object): bool;
  <<__Pure, __MaybeMutable>>
  public function isInstantiable(): bool;
  <<__Pure, __MaybeMutable>>
  public function isInterface(): bool;
  <<__Pure, __MaybeMutable>>
  public function isEnum(): bool;
  <<__Pure, __MaybeMutable>>
  public function getEnumUnderlyingType(): string;
  <<__Pure, __MaybeMutable>>
  public function isInternal(): bool;
  <<__Pure, __MaybeMutable>>
  public function isIterateable(): bool;
  /**
   * $class is string or ReflectionClass
   */
  <<__Pure, __MaybeMutable>>
  public function isSubclassOf(mixed $class): bool;
  <<__Pure, __MaybeMutable>>
  public function isTrait(): bool;
  <<__Pure, __MaybeMutable>>
  public function isUserDefined(): bool;
  public function newInstance(...$args);
  public function newInstanceArgs(Traversable<mixed> $args = varray[]);
  public function newInstanceWithoutConstructor();
  public function setStaticPropertyValue(string $name, mixed $value): void;
  <<__Pure, __MaybeMutable>>
  public function __toString(): string;
}

class ReflectionObject extends ReflectionClass {}

class ReflectionException extends Exception {}

abstract class ReflectionFunctionAbstract implements Reflector {

  public $name = '';

  // Methods
  <<__Pure, __MaybeMutable>>
  public function getName(): string;
  <<__Pure, __MaybeMutable>>
  public function inNamespace(): bool;
  <<__Pure, __MaybeMutable>>
  public function getNamespaceName(): string;
  <<__Pure, __MaybeMutable>>
  public function getShortName(): string;
  <<__Pure, __MaybeMutable>>
  public function isHack(): bool;
  <<__Pure, __MaybeMutable>>
  public function isInternal(): bool;
  <<__Pure, __MaybeMutable>>
  public function isClosure(): bool;
  <<__Pure, __MaybeMutable>>
  public function isGenerator(): bool;
  <<__Pure, __MaybeMutable>>
  public function isAsync(): bool;
  <<__Pure, __MaybeMutable>>
  public function isVariadic(): bool;
  <<__Pure, __MaybeMutable>>
  public function isUserDefined(): bool;
  <<__Pure, __MaybeMutable>>
  public function getFileName(): mixed; // string | false
  <<__Pure, __MaybeMutable>>
  public function getFile(): ReflectionFile;
  <<__Pure, __MaybeMutable>>
  public function getStartLine(): mixed; // int | false
  <<__Pure, __MaybeMutable>>
  public function getEndLine(): mixed; // int | false
  <<__Pure, __MaybeMutable>>
  public function getDocComment(): mixed; // string | false
  public function getStaticVariables(): darray<string, mixed>;
  <<__Pure, __MaybeMutable>>
  public function getReturnTypeText();
  <<__Pure, __MaybeMutable>>
  final public function getAttributes(): darray<string, varray<mixed>>;
  <<__Pure, __MaybeMutable>>
  final public function hasAttribute(string $name): bool;
  <<__Pure, __MaybeMutable>>
  final public function getAttribute(string $name): ?varray<mixed>;
  <<__Pure, __MaybeMutable>>
  public function getNumberOfParameters(): int;
  <<__Pure, __MaybeMutable>>
  public function getParameters(): varray<ReflectionParameter>;
  <<__Pure, __MaybeMutable>>
  public function getNumberOfRequiredParameters();
  <<__Pure, __MaybeMutable>>
  public function isDeprecated(): bool;
  <<__Pure, __MaybeMutable>>
  public function getExtension();
  <<__Pure, __MaybeMutable>>
  public function getExtensionName();
  final private function __clone();
  <<__Pure, __MaybeMutable>>
  public function hasReturnType(): bool;
  <<__Pure, __MaybeMutable>>
  public function getReturnType(): ?ReflectionType;
  <<__Pure, __MaybeMutable>>
  public function getReifiedTypeParamInfo(): varray<darray<string, bool>>;
}

class ReflectionFunction extends ReflectionFunctionAbstract implements Reflector {
  const IS_DEPRECATED = 262144;

  public $name = '';

  <<__Pure>>
  public function __construct($name);
  <<__Pure, __MaybeMutable>>
  public function __toString();
  public static function export($name, $return = null);
  <<__Pure, __MaybeMutable>>
  public function isDisabled();
  public function invoke(...$args);
  public function invokeArgs(varray $args);
  public function getClosure();
  <<__Pure, __MaybeMutable>>
  final public function getAttributeClass<T as HH\FunctionAttribute>(classname<T> $c): ?T;
}

class ReflectionMethod extends ReflectionFunctionAbstract implements Reflector {
  // Constants
  const IS_STATIC    = 1;
  const IS_PUBLIC    = 256;
  const IS_PROTECTED = 512;
  const IS_PRIVATE   = 1024;
  const IS_ABSTRACT  = 2;
  const IS_FINAL     = 4;

  public $name = '';
  public $class = '';

  public static function export(string $class, string $name, bool $return = false);
  <<__Pure>>
  public function __construct($class, $name = null);
  <<__Pure, __MaybeMutable>>
  public function __toString();
  <<__Pure, __MaybeMutable>>
  public function isPublic();
  <<__Pure, __MaybeMutable>>
  public function isPrivate();
  <<__Pure, __MaybeMutable>>
  public function isProtected();
  <<__Pure, __MaybeMutable>>
  public function isAbstract();
  <<__Pure, __MaybeMutable>>
  public function isFinal();
  <<__Pure, __MaybeMutable>>
  public function isStatic();
  <<__Pure, __MaybeMutable>>
  public function isConstructor();
  public function getClosure($object);
  <<__Pure, __MaybeMutable>>
  public function getModifiers();
  public function invoke($object, ...$args);
  public function invokeArgs($object, varray $args);
  <<__Pure, __MaybeMutable>>
  public function getDeclaringClass();
  <<__Pure, __MaybeMutable>>
  public function getPrototype();
  public function setAccessible(bool $accessible);
  <<__Pure, __MaybeMutable>>
  final public function getAttributeClass<T as HH\MethodAttribute>(classname<T> $c): ?T;
}

class ReflectionParameter implements Reflector {
  public $name = '';

  final private function __clone();
  public static function export($function, $parameter, $return = null);
  <<__Pure>>
  public function __construct($function, $parameter);
  <<__Pure, __MaybeMutable>>
  public function __toString();
  <<__Pure, __MaybeMutable>>
  public function getName();
  <<__Pure, __MaybeMutable>>
  public function isPassedByReference();
  <<__Pure, __MaybeMutable>>
  public function isInOut(): bool;
  <<__Pure, __MaybeMutable>>
  public function canBePassedByValue();
  <<__Pure, __MaybeMutable>>
  public function getDeclaringFunction();
  <<__Pure, __MaybeMutable>>
  public function getDeclaringClass();
  <<__Pure, __MaybeMutable>>
  public function getClass();
  <<__Pure, __MaybeMutable>>
  public function isArray();
  <<__Pure, __MaybeMutable>>
  public function isCallable();
  <<__Pure, __MaybeMutable>>
  public function allowsNull();
  <<__Pure, __MaybeMutable>>
  public function getPosition();
  <<__Pure, __MaybeMutable>>
  public function isOptional();
  <<__Pure, __MaybeMutable>>
  public function isDefaultValueAvailable();
  <<__Pure, __MaybeMutable>>
  public function getDefaultValue();
  <<__Pure, __MaybeMutable>>
  public function isDefaultValueConstant();
  <<__Pure, __MaybeMutable>>
  public function getDefaultValueConstantName();
  <<__Pure, __MaybeMutable>>
  public function getTypehintText();
  <<__Pure, __MaybeMutable>>
  public function getTypeText(): string;
  <<__Pure, __MaybeMutable>>
  public function getDefaultValueText();
  <<__Pure, __MaybeMutable>>
  public function isVariadic();
  <<__Pure, __MaybeMutable>>
  public function hasType(): bool;
  <<__Pure, __MaybeMutable>>
  public function getType(): ?ReflectionType;
  <<__Pure, __MaybeMutable>>
  final public function hasAttribute(string $name): bool;
  <<__Pure, __MaybeMutable>>
  final public function getAttribute(string $name): ?varray<mixed>;
  <<__Pure, __MaybeMutable>>
  final public function getAttributeClass<T as HH\ParameterAttribute>(classname<T> $c): ?T;
  <<__Pure, __MaybeMutable>>
  final public function getAttributes(): darray<string, varray<mixed>>;
}

class ReflectionProperty implements Reflector {
  const IS_STATIC = 1;
  const IS_PUBLIC = 256;
  const IS_PROTECTED = 512;
  const IS_PRIVATE = 1024;

  public $name = '';
  public $class = '';

  final private function __clone();
  public static function export($class, $name, $return = null);
  <<__Pure>>
  public function __construct($class, string $name);
  <<__Pure, __MaybeMutable>>
  public function __toString();
  <<__Pure, __MaybeMutable>>
  public function getName();
  public function getValue($object = null);
  public function setValue($object, $value = null);
  <<__Pure, __MaybeMutable>>
  public function isPublic();
  <<__Pure, __MaybeMutable>>
  public function isPrivate();
  <<__Pure, __MaybeMutable>>
  public function isProtected();
  <<__Pure, __MaybeMutable>>
  public function isStatic();
  <<__Pure, __MaybeMutable>>
  public function isDefault();
  <<__Pure, __MaybeMutable>>
  public function getModifiers();
  <<__Pure, __MaybeMutable>>
  public function getDeclaringClass();
  <<__Pure, __MaybeMutable>>
  public function getDocComment();
  public function setAccessible($accessible);
  <<__Pure, __MaybeMutable>>
  public function getTypeText();
  <<__Pure, __MaybeMutable>>
  final public function getAttributes(): darray<string, varray<mixed>>;
  <<__Pure, __MaybeMutable>>
  final public function hasAttribute(string $name): bool;
  <<__Pure, __MaybeMutable>>
  final public function getAttribute(string $name): ?varray<mixed>;
}

class ReflectionExtension implements Reflector {

  public $name = '';

  final private function __clone();
  public static function export($name, $return = false);
  <<__Pure>>
  public function __construct($name);
  <<__Pure, __MaybeMutable>>
  public function __toString();
  <<__Pure, __MaybeMutable>>
  public function getName();
  <<__Pure, __MaybeMutable>>
  public function getVersion();
  <<__Pure, __MaybeMutable>>
  public function getFunctions();
  <<__Pure, __MaybeMutable>>
  public function getConstants();
  <<__Pure, __MaybeMutable>>
  public function getINIEntries();
  <<__Pure, __MaybeMutable>>
  public function getClasses();
  <<__Pure, __MaybeMutable>>
  public function getClassNames();
  <<__Pure, __MaybeMutable>>
  public function info();
}

class ReflectionTypeConstant implements Reflector {

  final private function __clone();
  public static function export($class, string $name, $return = null);
  <<__Pure>>
  public function __construct($class, string $name);
  <<__Pure, __MaybeMutable>>
  public function __toString(): string;
  <<__Pure, __MaybeMutable>>
  public function getName(): string;
  <<__Pure, __MaybeMutable>>
  public function isAbstract(): bool;
  <<__Pure, __MaybeMutable>>
  public function getDeclaringClass(): ReflectionClass;
  <<__Pure, __MaybeMutable>>
  public function getClass(): ReflectionClass;
  <<__Pure, __MaybeMutable>>
  public function getAssignedTypeText(): ?string;
}

class ReflectionTypeAlias implements Reflector {
  final private function __clone();
  <<__Pure>>
  final public function __construct(string $name);
  <<__Pure, __MaybeMutable>>
  public function __toString(): string;
  <<__Pure, __MaybeMutable>>
  public function getTypeStructure(): darray;
  <<__Pure, __MaybeMutable>>
  public function getResolvedTypeStructure(): darray;
  <<__Pure, __MaybeMutable>>
  public function getAssignedTypeText(): string;
  <<__Pure, __MaybeMutable>>
  public function getName(): string;
  <<__Pure, __MaybeMutable>>
  public function getFileName(): string;
  <<__Pure, __MaybeMutable>>
  public function getFile(): ReflectionFile;
  <<__Pure, __MaybeMutable>>
  final public function getAttributes(): darray<string, varray<mixed>>;
  <<__Pure, __MaybeMutable>>
  final public function hasAttribute(string $name): bool;
  <<__Pure, __MaybeMutable>>
  final public function getAttribute(string $name): ?varray<mixed>;
  <<__Pure, __MaybeMutable>>
  final public function getAttributeClass<T as HH\TypeAliasAttribute>(classname<T> $c): ?T;
}

class ReflectionType {
  final private function __clone();
  <<__Pure>>
  public function __construct(?Reflector $param_or_ret = null,
                              darray $type_hint_info = darray[]);
  <<__Pure, __MaybeMutable>>
  public function allowsNull(): bool;
  <<__Pure, __MaybeMutable>>
  public function isBuiltin(): bool;
  <<__Pure, __MaybeMutable>>
  public function __toString(): string;
}

class ReflectionFile implements Reflector {
  final private function __clone();
  <<__Pure>>
  final public function __construct(string $name);
  <<__Pure, __MaybeMutable>>
  public function __toString(): string;
  <<__Pure, __MaybeMutable>>
  public function getName(): string;
  <<__Pure, __MaybeMutable>>
  final public function getAttributes(): darray<string, varray<mixed>>;
  <<__Pure, __MaybeMutable>>
  final public function hasAttribute(string $name): bool;
  <<__Pure, __MaybeMutable>>
  final public function getAttribute(string $name): ?varray<mixed>;
  <<__Pure, __MaybeMutable>>
  final public function getAttributeClass<T as HH\FileAttribute>(classname<T> $c): ?T;
}
