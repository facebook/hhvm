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
  <<__Rx, __MaybeMutable>>
  public function __toString();
}

class Reflection  {
  <<__Rx>>
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

  <<__Rx>>
  public function __construct(<<__MaybeMutable>> mixed $argument);
  public static function export(mixed $argument, bool $return = false): ?string;
  <<__Rx, __MaybeMutable>>
  public function getConstant(string $name): mixed;
  <<__Rx, __MaybeMutable>>
  public function getConstants(): darray<string, mixed>;
  <<__Rx, __MaybeMutable>>
  public function getAbstractConstantNames(): darray<string, string>;
  <<__Rx, __MaybeMutable>>
  public function getTypeConstant(string $name): ReflectionTypeConstant;
  <<__Rx, __MaybeMutable>>
  public function getTypeConstants(): varray<ReflectionTypeConstant>;
  <<__Rx, __MaybeMutable>>
  public function getConstructor(): ?ReflectionMethod;
  <<__Rx, __MaybeMutable>>
  public function getDefaultProperties(): darray<string, mixed>;
  /**
   * Returns string or false
   */
  <<__Rx, __MaybeMutable>>
  public function getDocComment(): mixed;
  <<__Rx, __MaybeMutable>>
  public function getEndLine(): int;
  <<__Rx, __MaybeMutable>>
  public function getExtension(): ?ReflectionExtension;
  <<__Rx, __MaybeMutable>>
  public function getExtensionName(): string;
  /**
   * Returns string or false
   */
  <<__Rx, __MaybeMutable>>
  public function getFileName(): mixed;
  <<__Rx, __MaybeMutable>>
  public function getFile(): ReflectionFile;
  <<__Rx, __MaybeMutable>>
  public function getInterfaceNames(): varray<string>;
  <<__Rx, __MaybeMutable>>
  public function getInterfaces(): darray<string, ReflectionClass>;
  <<__Rx, __MaybeMutable>>
  final public function getAttributes(): darray<string, varray<mixed>>;
  <<__Rx, __MaybeMutable>>
  final public function getAttribute(string $name): ?varray<mixed>;
  <<__Rx, __MaybeMutable>>
  final public function getAttributeClass<T as HH\ClassLikeAttribute>(classname<T> $c): ?T;
  <<__Rx, __MaybeMutable>>
  public function getMethod(string $name): ReflectionMethod;
  <<__Rx, __MaybeMutable>>
  public function getMethods(?int $filter = null): varray<ReflectionMethod>;
  <<__Rx, __MaybeMutable>>
  public function getModifiers(): int;
  <<__Rx, __MaybeMutable>>
  public function getName(): string;
  <<__Rx, __MaybeMutable>>
  public function getNamespaceName(): string;
  /**
   * Returns ReflectionClass or false
   */
  <<__Rx, __MaybeMutable>>
  public function getParentClass(): mixed;
  <<__Rx, __MaybeMutable>>
  public function getProperties(int $filter = 0xFFFF): varray<ReflectionProperty>;
  <<__Rx, __MaybeMutable>>
  public function getProperty(string $name): ReflectionProperty;
  <<__Rx, __MaybeMutable>>
  public function getRequirementNames(): varray<string>;
  <<__Rx, __MaybeMutable>>
  public function getRequirements(): darray<string, ReflectionClass>;
  <<__Rx, __MaybeMutable>>
  public function getShortName(): string;
  <<__Rx, __MaybeMutable>>
  public function getStartLine(): int;
  public function getStaticProperties(): darray<string, mixed>;
  public function getStaticPropertyValue(string $name, mixed $def_value = null): mixed;
  <<__Rx, __MaybeMutable>>
  public function getTraitAliases(): darray<string, string>;
  <<__Rx, __MaybeMutable>>
  public function getTraitNames(): varray<string>;
  <<__Rx, __MaybeMutable>>
  public function getTraits(): darray<string, ReflectionClass>;
  <<__Rx, __MaybeMutable>>
  public function hasConstant(string $name): bool;
  <<__Rx, __MaybeMutable>>
  public function hasMethod(string $name): bool;
  <<__Rx, __MaybeMutable>>
  public function hasProperty(string $name): bool;
  <<__Rx, __MaybeMutable>>
  public function hasTypeConstant(string $name): bool;
  <<__Rx, __MaybeMutable>>
  public function implementsInterface(string $interface): bool;
  <<__Rx, __MaybeMutable>>
  public function inNamespace(): bool;
  <<__Rx, __MaybeMutable>>
  public function isAbstract(): bool;
  <<__Rx, __MaybeMutable>>
  public function isCloneable(): bool;
  <<__Rx, __MaybeMutable>>
  public function isFinal(): bool;
  <<__Rx, __MaybeMutable>>
  public function isInstance(<<__MaybeMutable>> mixed $object): bool;
  <<__Rx, __MaybeMutable>>
  public function isInstantiable(): bool;
  <<__Rx, __MaybeMutable>>
  public function isInterface(): bool;
  <<__Rx, __MaybeMutable>>
  public function isEnum(): bool;
  <<__Rx, __MaybeMutable>>
  public function getEnumUnderlyingType(): string;
  <<__Rx, __MaybeMutable>>
  public function isInternal(): bool;
  <<__Rx, __MaybeMutable>>
  public function isIterateable(): bool;
  /**
   * $class is string or ReflectionClass
   */
  <<__Rx, __MaybeMutable>>
  public function isSubclassOf(mixed $class): bool;
  <<__Rx, __MaybeMutable>>
  public function isTrait(): bool;
  <<__Rx, __MaybeMutable>>
  public function isUserDefined(): bool;
  public function newInstance(...$args);
  public function newInstanceArgs(Traversable<mixed> $args = varray[]);
  public function newInstanceWithoutConstructor();
  public function setStaticPropertyValue(string $name, mixed $value): void;
  <<__Rx, __MaybeMutable>>
  public function __toString(): string;
}

class ReflectionObject extends ReflectionClass {}

class ReflectionException extends Exception {}

abstract class ReflectionFunctionAbstract implements Reflector {

  public $name = '';

  // Methods
  <<__Rx, __MaybeMutable>>
  public function getName(): string;
  <<__Rx, __MaybeMutable>>
  public function inNamespace(): bool;
  <<__Rx, __MaybeMutable>>
  public function getNamespaceName(): string;
  <<__Rx, __MaybeMutable>>
  public function getShortName(): string;
  <<__Rx, __MaybeMutable>>
  public function isHack(): bool;
  <<__Rx, __MaybeMutable>>
  public function isInternal(): bool;
  <<__Rx, __MaybeMutable>>
  public function isClosure(): bool;
  <<__Rx, __MaybeMutable>>
  public function isGenerator(): bool;
  <<__Rx, __MaybeMutable>>
  public function isAsync(): bool;
  <<__Rx, __MaybeMutable>>
  public function isVariadic(): bool;
  <<__Rx, __MaybeMutable>>
  public function isUserDefined(): bool;
  <<__Rx, __MaybeMutable>>
  public function getFileName(): mixed; // string | false
  <<__Rx, __MaybeMutable>>
  public function getFile(): ReflectionFile;
  <<__Rx, __MaybeMutable>>
  public function getStartLine(): mixed; // int | false
  <<__Rx, __MaybeMutable>>
  public function getEndLine(): mixed; // int | false
  <<__Rx, __MaybeMutable>>
  public function getDocComment(): mixed; // string | false
  public function getStaticVariables(): darray<string, mixed>;
  <<__Rx, __MaybeMutable>>
  public function getReturnTypeText();
  <<__Rx, __MaybeMutable>>
  final public function getAttributes(): darray<string, varray<mixed>>;
  <<__Rx, __MaybeMutable>>
  final public function getAttribute(string $name): ?varray<mixed>;
  <<__Rx, __MaybeMutable>>
  public function getNumberOfParameters(): int;
  <<__Rx, __MaybeMutable>>
  public function getParameters(): varray<ReflectionParameter>;
  <<__Rx, __MaybeMutable>>
  public function getNumberOfRequiredParameters();
  <<__Rx, __MaybeMutable>>
  public function isDeprecated(): bool;
  <<__Rx, __MaybeMutable>>
  public function getExtension();
  <<__Rx, __MaybeMutable>>
  public function getExtensionName();
  final private function __clone();
  <<__Rx, __MaybeMutable>>
  public function hasReturnType(): bool;
  <<__Rx, __MaybeMutable>>
  public function getReturnType(): ?ReflectionType;
  <<__Rx, __MaybeMutable>>
  public function getReifiedTypeParamInfo(): varray<darray<string, bool>>;
}

class ReflectionFunction extends ReflectionFunctionAbstract implements Reflector {
  const IS_DEPRECATED = 262144;

  public $name = '';

  <<__Rx>>
  public function __construct($name);
  <<__Rx, __MaybeMutable>>
  public function __toString();
  public static function export($name, $return = null);
  <<__Rx, __MaybeMutable>>
  public function isDisabled();
  public function invoke(...$args);
  public function invokeArgs(varray $args);
  public function getClosure();
  <<__Rx, __MaybeMutable>>
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
  <<__Rx>>
  public function __construct($class, $name = null);
  <<__Rx, __MaybeMutable>>
  public function __toString();
  <<__Rx, __MaybeMutable>>
  public function isPublic();
  <<__Rx, __MaybeMutable>>
  public function isPrivate();
  <<__Rx, __MaybeMutable>>
  public function isProtected();
  <<__Rx, __MaybeMutable>>
  public function isAbstract();
  <<__Rx, __MaybeMutable>>
  public function isFinal();
  <<__Rx, __MaybeMutable>>
  public function isStatic();
  <<__Rx, __MaybeMutable>>
  public function isConstructor();
  public function getClosure($object);
  <<__Rx, __MaybeMutable>>
  public function getModifiers();
  public function invoke($object, ...$args);
  public function invokeArgs($object, varray $args);
  <<__Rx, __MaybeMutable>>
  public function getDeclaringClass();
  <<__Rx, __MaybeMutable>>
  public function getPrototype();
  public function setAccessible(bool $accessible);
  <<__Rx, __MaybeMutable>>
  final public function getAttributeClass<T as HH\MethodAttribute>(classname<T> $c): ?T;
}

class ReflectionParameter implements Reflector {
  public $name = '';

  final private function __clone();
  public static function export($function, $parameter, $return = null);
  <<__Rx>>
  public function __construct($function, $parameter);
  <<__Rx, __MaybeMutable>>
  public function __toString();
  <<__Rx, __MaybeMutable>>
  public function getName();
  <<__Rx, __MaybeMutable>>
  public function isPassedByReference();
  <<__Rx, __MaybeMutable>>
  public function isInOut(): bool;
  <<__Rx, __MaybeMutable>>
  public function canBePassedByValue();
  <<__Rx, __MaybeMutable>>
  public function getDeclaringFunction();
  <<__Rx, __MaybeMutable>>
  public function getDeclaringClass();
  <<__Rx, __MaybeMutable>>
  public function getClass();
  <<__Rx, __MaybeMutable>>
  public function isArray();
  <<__Rx, __MaybeMutable>>
  public function isCallable();
  <<__Rx, __MaybeMutable>>
  public function allowsNull();
  <<__Rx, __MaybeMutable>>
  public function getPosition();
  <<__Rx, __MaybeMutable>>
  public function isOptional();
  <<__Rx, __MaybeMutable>>
  public function isDefaultValueAvailable();
  <<__Rx, __MaybeMutable>>
  public function getDefaultValue();
  <<__Rx, __MaybeMutable>>
  public function isDefaultValueConstant();
  <<__Rx, __MaybeMutable>>
  public function getDefaultValueConstantName();
  <<__Rx, __MaybeMutable>>
  public function getTypehintText();
  <<__Rx, __MaybeMutable>>
  public function getTypeText(): string;
  <<__Rx, __MaybeMutable>>
  public function getDefaultValueText();
  <<__Rx, __MaybeMutable>>
  public function isVariadic();
  <<__Rx, __MaybeMutable>>
  public function hasType(): bool;
  <<__Rx, __MaybeMutable>>
  public function getType(): ?ReflectionType;
  <<__Rx, __MaybeMutable>>
  final public function getAttribute(string $name): ?varray<mixed>;
  <<__Rx, __MaybeMutable>>
  final public function getAttributeClass<T as HH\ParameterAttribute>(classname<T> $c): ?T;
  <<__Rx, __MaybeMutable>>
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
  <<__Rx>>
  public function __construct($class, string $name);
  <<__Rx, __MaybeMutable>>
  public function __toString();
  <<__Rx, __MaybeMutable>>
  public function getName();
  public function getValue($object = null);
  public function setValue($object, $value = null);
  <<__Rx, __MaybeMutable>>
  public function isPublic();
  <<__Rx, __MaybeMutable>>
  public function isPrivate();
  <<__Rx, __MaybeMutable>>
  public function isProtected();
  <<__Rx, __MaybeMutable>>
  public function isStatic();
  <<__Rx, __MaybeMutable>>
  public function isDefault();
  <<__Rx, __MaybeMutable>>
  public function getModifiers();
  <<__Rx, __MaybeMutable>>
  public function getDeclaringClass();
  <<__Rx, __MaybeMutable>>
  public function getDocComment();
  public function setAccessible($accessible);
  <<__Rx, __MaybeMutable>>
  public function getTypeText();
  public function getAttributes(): darray<string, varray<mixed>>;
  public function getAttribute(string $name): ?varray<mixed>;
}

class ReflectionExtension implements Reflector {

  public $name = '';

  final private function __clone();
  public static function export($name, $return = false);
  <<__Rx>>
  public function __construct($name);
  <<__Rx, __MaybeMutable>>
  public function __toString();
  <<__Rx, __MaybeMutable>>
  public function getName();
  <<__Rx, __MaybeMutable>>
  public function getVersion();
  <<__Rx, __MaybeMutable>>
  public function getFunctions();
  <<__Rx, __MaybeMutable>>
  public function getConstants();
  <<__Rx, __MaybeMutable>>
  public function getINIEntries();
  <<__Rx, __MaybeMutable>>
  public function getClasses();
  <<__Rx, __MaybeMutable>>
  public function getClassNames();
  <<__Rx, __MaybeMutable>>
  public function info();
}

class ReflectionTypeConstant implements Reflector {

  final private function __clone();
  public static function export($class, string $name, $return = null);
  <<__Rx>>
  public function __construct($class, string $name);
  <<__Rx, __MaybeMutable>>
  public function __toString(): string;
  <<__Rx, __MaybeMutable>>
  public function getName(): string;
  <<__Rx, __MaybeMutable>>
  public function isAbstract(): bool;
  <<__Rx, __MaybeMutable>>
  public function getDeclaringClass(): ReflectionClass;
  <<__Rx, __MaybeMutable>>
  public function getClass(): ReflectionClass;
  <<__Rx, __MaybeMutable>>
  public function getAssignedTypeText(): ?string;
}

class ReflectionTypeAlias implements Reflector {
  final private function __clone();
  <<__Rx>>
  final public function __construct(string $name);
  <<__Rx, __MaybeMutable>>
  public function __toString(): string;
  <<__Rx, __MaybeMutable>>
  public function getTypeStructure(): darray;
  <<__Rx, __MaybeMutable>>
  public function getResolvedTypeStructure(): darray;
  <<__Rx, __MaybeMutable>>
  public function getAssignedTypeText(): string;
  <<__Rx, __MaybeMutable>>
  public function getName(): string;
  <<__Rx, __MaybeMutable>>
  public function getFileName(): string;
  <<__Rx, __MaybeMutable>>
  public function getFile(): ReflectionFile;
  <<__Rx, __MaybeMutable>>
  final public function getAttributes(): darray<string, varray<mixed>>;
  <<__Rx, __MaybeMutable>>
  final public function getAttribute(string $name): ?varray<mixed>;
  <<__Rx, __MaybeMutable>>
  final public function getAttributeClass<T as HH\TypeAliasAttribute>(classname<T> $c): ?T;
}

class ReflectionType {
  final private function __clone();
  <<__Rx>>
  public function __construct(?Reflector $param_or_ret = null,
                              darray $type_hint_info = darray[]);
  <<__Rx, __MaybeMutable>>
  public function allowsNull(): bool;
  <<__Rx, __MaybeMutable>>
  public function isBuiltin(): bool;
  <<__Rx, __MaybeMutable>>
  public function __toString(): string;
}

class ReflectionFile implements Reflector {
  final private function __clone();
  <<__Rx>>
  final public function __construct(string $name);
  <<__Rx, __MaybeMutable>>
  public function __toString(): string;
  <<__Rx, __MaybeMutable>>
  public function getName(): string;
  <<__Rx, __MaybeMutable>>
  final public function getAttributes(): darray<string, varray<mixed>>;
  <<__Rx, __MaybeMutable>>
  final public function getAttribute(string $name): ?varray<mixed>;
  <<__Rx, __MaybeMutable>>
  final public function getAttributeClass<T as HH\FileAttribute>(classname<T> $c): ?T;
}
