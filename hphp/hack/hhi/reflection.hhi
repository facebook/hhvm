<?hh // decl /* -*- mode: php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

interface Reflector {
  public function __toString();
}

class Reflection  {
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

  public function __construct(mixed $argument);
  public static function export(mixed $argument, bool $return = false): ?string;
  public function getConstant(string $name): mixed;
  public function getConstants(): array<string, mixed>;
  public function getAbstractConstantNames(): array<string, string>;
  public function getTypeConstant(string $name): ReflectionTypeConstant;
  public function getTypeConstants(): array<ReflectionTypeConstant>;
  public function getConstructor(): ?ReflectionMethod;
  public function getDefaultProperties(): array<string, mixed>;
  /**
   * Returns string or false
   */
  public function getDocComment(): mixed;
  public function getEndLine(): int;
  public function getExtension(): ?ReflectionExtension;
  public function getExtensionName(): string;
  /**
   * Returns string or false
   */
  public function getFileName(): mixed;
  public function getInterfaceNames(): array<string>;
  public function getInterfaces(): array<string, ReflectionClass>;
  final public function getAttributes(): array<string, array<mixed>>;
  final public function getAttribute(string $name): ?array<mixed>;
  final public function getAttributesRecursive(): array<string, array<mixed>>;
  final public function getAttributeRecursive(string $name): ?array<mixed>;
  public function getMethod(string $name): ReflectionMethod;
  public function getMethods(?int $filter = null): array<ReflectionMethod>;
  public function getModifiers(): int;
  public function getName(): string;
  public function getNamespaceName(): string;
  /**
   * Returns ReflectionClass or false
   */
  public function getParentClass(): mixed;
  public function getProperties(int $filter = 0xFFFF): array<ReflectionProperty>;
  public function getProperty(string $name): ReflectionProperty;
  public function getRequirementNames(): array<string>;
  public function getRequirements(): array<string, ReflectionClass>;
  public function getShortName(): string;
  public function getStartLine(): int;
  public function getStaticProperties(): array<string, ReflectionProperty>;
  public function getStaticPropertyValue(string $name, mixed $def_value = null): mixed;
  public function getTraitAliases(): array<string, string>;
  public function getTraitNames(): array<string>;
  public function getTraits(): array<string, ReflectionClass>;
  public function hasConstant(string $name): bool;
  public function hasMethod(string $name): bool;
  public function hasProperty(string $name): bool;
  public function hasTypeConstant(string $name): bool;
  public function implementsInterface(string $interface): bool;
  public function inNamespace(): bool;
  public function isAbstract(): bool;
  public function isCloneable(): bool;
  public function isFinal(): bool;
  public function isInstance(mixed $object): bool;
  public function isInstantiable(): bool;
  public function isInterface(): bool;
  public function isEnum(): bool;
  public function isInternal(): bool;
  public function isIterateable(): bool;
  /**
   * $class is string or ReflectionClass
   */
  public function isSubclassOf(mixed $class): bool;
  public function isTrait(): bool;
  public function isUserDefined(): bool;
  public function newInstance(...$args);
  public function newInstanceArgs(Traversable<mixed> $args = array());
  public function newInstanceWithoutConstructor();
  public function setStaticPropertyValue(string $name, string $value): void;
  public function __toString(): string;
}

class ReflectionObject extends ReflectionClass {}

class ReflectionException extends Exception {}

abstract class ReflectionFunctionAbstract implements Reflector {

  public $name = '';

  // Methods
  public function getName(): string;
  public function inNamespace(): bool;
  public function getNamespaceName(): string;
  public function getShortName(): string;
  public function isHack(): bool;
  public function isInternal(): bool;
  public function isClosure(): bool;
  public function isGenerator(): bool;
  public function isAsync(): bool;
  public function isVariadic(): bool;
  public function isUserDefined(): bool;
  public function getFileName(): mixed; // string | false
  public function getStartLine(): mixed; // int | false
  public function getEndLine(): mixed; // int | false
  public function getDocComment(): mixed; // string | false
  public function getStaticVariables(): array<string, mixed>;
  public function returnsReference(): bool;
  public function getReturnTypeText();
  final public function getAttributes(): array<string, array<mixed>>;
  final public function getAttribute(string $name): ?array<mixed>;
  public function getNumberOfParameters(): int;
  public function getParameters(): array<ReflectionParameter>;
  public function getNumberOfRequiredParameters();
  public function isDeprecated(): bool;
  public function getExtension();
  public function getExtensionName();
  final private function __clone();
}

class ReflectionFunction extends ReflectionFunctionAbstract implements Reflector {
  const IS_DEPRECATED = 262144;

  public $name = '';

  public function __construct($name);
  public function __toString();
  public static function export($name, $return = null);
  public function isDisabled();
  public function invoke(...);
  public function invokeArgs(array $args);
  public function getClosure();
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

  public static function export($class, $name, $return = false);
  public function __construct($class, $name = null);
  public function __toString();
  public function isPublic();
  public function isPrivate();
  public function isProtected();
  public function isAbstract();
  public function isFinal();
  public function isStatic();
  public function isConstructor();
  public function isDestructor();
  public function getClosure($object);
  public function getModifiers();
  public function invoke($object, ...);
  public function invokeArgs($object, array $args);
  public function getDeclaringClass();
  public function getPrototype();
  public function setAccessible($accessible);
  final public function getAttributesRecursive(): array<string, array<mixed>>;
  final public function getAttributeRecursive(string $name): ?array<mixed>;
}

class ReflectionParameter implements Reflector {
  public $name = '';

  final private function __clone();
  public static function export($function, $parameter, $return = null);
  public function __construct($function, $parameter);
  public function __toString();
  public function getName();
  public function isPassedByReference();
  public function canBePassedByValue();
  public function getDeclaringFunction();
  public function getDeclaringClass();
  public function getClass();
  public function isArray();
  public function isCallable();
  public function allowsNull();
  public function getPosition();
  public function isOptional();
  public function isDefaultValueAvailable();
  public function getDefaultValue();
  public function isDefaultValueConstant();
  public function getDefaultValueConstantName();
  public function getTypehintText();
  public function getTypeText(): string;
  public function getDefaultValueText();
  public function isVariadic();
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
  public function __construct($class, $name);
  public function __toString();
  public function getName();
  public function getValue($object = null);
  public function setValue($object, $value = null);
  public function isPublic();
  public function isPrivate();
  public function isProtected();
  public function isStatic();
  public function isDefault();
  public function getModifiers();
  public function getDeclaringClass();
  public function getDocComment();
  public function setAccessible($accessible);
  public function getTypeText();
}

class ReflectionExtension implements Reflector {

  public $name = '';

  final private function __clone();
  public static function export($name, $return = false);
  public function __construct($name);
  public function __toString();
  public function getName();
  public function getVersion();
  public function getFunctions();
  public function getConstants();
  public function getINIEntries();
  public function getClasses();
  public function getClassNames();
  public function getDependencies();
  public function info();
  public function isPersistent();
  public function isTemporary();
}

class ReflectionZendExtension implements Reflector {

  public $name = '';

  final private function __clone();
  public static function export($name, $return = null);
  public function __construct($name);
  public function __toString();
  public function getName();
  public function getVersion();
  public function getAuthor();
  public function getURL();
  public function getCopyright();
}

class ReflectionTypeConstant implements Reflector {

  final private function __clone();
  public static function export($class, string $name, $return = null);
  public function __construct($class, string $name);
  public function __toString(): string;
  public function getName(): string;
  public function isAbstract(): bool;
  public function getDeclaringClass(): ReflectionClass;
  public function getAssignedTypeText(): ?string;
}
