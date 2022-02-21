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
  public function __toString()[];
}

class Reflection  {
  public static function getModifierNames($modifiers)[];
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

  public function __construct(mixed $argument)[];
  public static function export(mixed $argument, bool $return = false): ?string;
  public function getConstant(string $name)[]: mixed;
  public function getConstants()[]: darray<string, mixed>;
  public function getAbstractConstantNames()[]: darray<string, string>;
  public function getTypeConstant(string $name)[]: ReflectionTypeConstant;
  public function getTypeConstants()[]: varray<ReflectionTypeConstant>;
  public function getConstructor()[]: ?ReflectionMethod;
  public function getDefaultProperties()[]: darray<string, mixed>;
  /**
   * Returns string or false
   */
  public function getDocComment()[]: mixed;
  public function getEndLine()[]: int;
  public function getExtension()[]: ?ReflectionExtension;
  public function getExtensionName()[]: string;
  /**
   * Returns string or false
   */
  public function getFileName()[]: mixed;
  public function getFile()[]: ReflectionFile;
  public function getInterfaceNames()[]: varray<string>;
  public function getInterfaces()[]: darray<string, ReflectionClass>;
  final public function getAttributes()[]: darray<string, varray<mixed>>;
  final public function hasAttribute(string $name)[]: bool;
  final public function getAttribute(string $name)[]: ?varray<mixed>;
  final public function getAttributeClass<T as HH\ClassLikeAttribute>(classname<T> $c)[]: ?T;
  public function getMethod(string $name)[]: ReflectionMethod;
  public function getMethods(?int $filter = null)[]: varray<ReflectionMethod>;
  public function getModifiers()[]: int;
  public function getName()[]: string;
  public function getNamespaceName()[]: string;
  /**
   * Returns ReflectionClass or false
   */
  public function getParentClass()[]: mixed;
  public function getProperties(int $filter = 0xFFFF)[]: varray<ReflectionProperty>;
  public function getProperty(string $name)[]: ReflectionProperty;
  public function getRequirementNames()[]: varray<string>;
  public function getRequirements()[]: darray<string, ReflectionClass>;
  public function getShortName()[]: string;
  public function getStartLine()[]: int;
  public function getStaticProperties(): darray<string, mixed>;
  public function getStaticPropertyValue(string $name, mixed $def_value = null): mixed;
  public function getTraitAliases()[]: darray<string, string>;
  public function getTraitNames()[]: varray<string>;
  public function getTraits()[]: darray<string, ReflectionClass>;
  public function hasConstant(string $name)[]: bool;
  public function hasMethod(string $name)[]: bool;
  public function hasProperty(string $name)[]: bool;
  public function hasTypeConstant(string $name)[]: bool;
  public function implementsInterface(string $interface)[]: bool;
  public function inNamespace()[]: bool;
  public function isAbstract()[]: bool;
  public function isCloneable()[]: bool;
  public function isFinal()[]: bool;
  public function isInstance(mixed $object)[]: bool;
  public function isInstantiable()[]: bool;
  public function isInterface()[]: bool;
  public function isEnum()[]: bool;
  public function getEnumUnderlyingType()[]: string;
  public function isInternal()[]: bool;
  public function isIterateable()[]: bool;
  /**
   * $class is string or ReflectionClass
   */
  public function isSubclassOf(mixed $class)[]: bool;
  public function isTrait()[]: bool;
  public function isUserDefined()[]: bool;
  public function newInstance(...$args);
  public function newInstanceArgs(Traversable<mixed> $args = varray[])[defaults];
  public function newInstanceWithoutConstructor()[];
  public function setStaticPropertyValue(string $name, mixed $value): void;
  public function __toString()[]: string;
  public function getReifiedTypeParamInfo()[]: varray<shape(
    'is_reified' => bool,
    'is_soft' => bool,
    'is_warn' => bool,
  )>;
}

class ReflectionObject extends ReflectionClass {}

class ReflectionException extends Exception {
  use ExceptionWithPureGetMessageTrait;
}

abstract class ReflectionFunctionAbstract implements Reflector {

  public $name = '';

  // Methods
  public function getName()[]: string;
  public function inNamespace()[]: bool;
  public function getNamespaceName()[]: string;
  public function getShortName()[]: string;
  public function isHack()[]: bool;
  public function isInternal()[]: bool;
  public function isClosure()[]: bool;
  public function isGenerator()[]: bool;
  public function returnsReadonly()[]: bool;
  public function isAsync()[]: bool;
  public function isVariadic()[]: bool;
  public function isUserDefined()[]: bool;
  public function getFileName()[]: mixed; // string | false
  public function getFile()[]: ReflectionFile;
  public function getStartLine()[]: mixed; // int | false
  public function getEndLine()[]: mixed; // int | false
  public function getDocComment()[]: mixed; // string | false
  public function getStaticVariables(): darray<string, mixed>;
  public function getReturnTypeText()[];
  final public function getAttributes()[]: darray<string, varray<mixed>>;
  final public function hasAttribute(string $name)[]: bool;
  final public function getAttribute(string $name)[]: ?varray<mixed>;
  public function getNumberOfParameters()[]: int;
  public function getParameters()[]: varray<ReflectionParameter>;
  public function getNumberOfRequiredParameters()[];
  public function isDeprecated()[]: bool;
  public function getExtension()[];
  public function getExtensionName()[];
  private function __clone();
  public function hasReturnType()[]: bool;
  public function getReturnType()[]: ?ReflectionType;
  public function getReifiedTypeParamInfo()[]: varray<darray<string, bool>>;
  public function getCoeffects()[]: vec<string>;
}

class ReflectionFunction extends ReflectionFunctionAbstract implements Reflector {
  const int IS_DEPRECATED = 262144;

  public $name = '';

  public function __construct($name)[];
  public function __toString()[];
  public static function export($name, $return = null);
  public function isDisabled()[];
  public function invoke(...$args);
  public function invokeArgs(varray $args);
  public function getClosure();
  final public function getAttributeClass<T as HH\FunctionAttribute>(classname<T> $c)[]: ?T;
}

class ReflectionMethod extends ReflectionFunctionAbstract implements Reflector {
  // Constants
  const int IS_STATIC    = 1;
  const int IS_PUBLIC    = 256;
  const int IS_PROTECTED = 512;
  const int IS_PRIVATE   = 1024;
  const int IS_ABSTRACT  = 2;
  const int IS_FINAL     = 4;

  public $name = '';
  public $class = '';

  public static function export(string $class, string $name, bool $return = false);
  public function __construct($class, $name = null)[];
  public function __toString()[];
  public function isPublic()[];
  public function isPrivate()[];
  public function isProtected()[];
  public function isAbstract()[];
  public function isFinal()[];
  public function isStatic()[];
  public function isReadonly()[];
  public function isConstructor()[];
  public function getClosure($object);
  public function getModifiers()[];
  public function invoke($object, ...$args);
  public function invokeArgs($object, varray $args);
  public function getDeclaringClass()[];
  public function getPrototype()[];
  public function setAccessible(bool $accessible)[write_props]: void;
  final public function getAttributeClass<T as HH\MethodAttribute>(classname<T> $c)[]: ?T;
}

class ReflectionParameter implements Reflector {
  public $name = '';

  private function __clone();
  public static function export($function, $parameter, $return = null);
  public function __construct($function, $parameter, $info = null)[];
  public function __toString()[];
  public function getName()[];
  public function isPassedByReference()[];
  public function isInOut()[]: bool;
  public function isReadonly()[]: bool;
  public function canBePassedByValue()[];
  public function getDeclaringFunction()[];
  public function getDeclaringClass()[];
  public function getClass()[];
  public function isArray()[];
  public function isCallable()[];
  public function allowsNull()[];
  public function getPosition()[];
  public function isOptional()[];
  public function isDefaultValueAvailable()[];
  public function getDefaultValue()[];
  public function isDefaultValueConstant()[];
  public function getDefaultValueConstantName()[];
  public function getTypehintText()[];
  public function getTypeText()[]: string;
  public function getDefaultValueText()[];
  public function isVariadic()[];
  public function hasType()[]: bool;
  public function getType()[]: ?ReflectionType;
  final public function hasAttribute(string $name)[]: bool;
  final public function getAttribute(string $name)[]: ?varray<mixed>;
  final public function getAttributeClass<T as HH\ParameterAttribute>(classname<T> $c)[]: ?T;
  final public function getAttributes()[]: darray<string, varray<mixed>>;
}

class ReflectionProperty implements Reflector {
  const int IS_STATIC = 1;
  const int IS_PUBLIC = 256;
  const int IS_PROTECTED = 512;
  const int IS_PRIVATE = 1024;

  public $name = '';
  public $class = '';

  private function __clone();
  public static function export($class, $name, $return = null);
  public function __construct($class, string $name)[];
  public function __toString()[];
  public function getName()[];
  public function getValue($obj = null)[read_globals];
  public function setValue($obj, $value = null)[globals, write_props];
  public function isPublic()[];
  public function isPrivate()[];
  public function isProtected()[];
  public function isStatic()[];
  public function isReadonly()[];
  public function isDefault()[];
  public function getModifiers()[];
  public function getDeclaringClass()[];
  public function getDocComment()[];
  public function setAccessible(bool $accessible)[write_props]: void;
  public function getTypeText()[];
  final public function getAttributes()[]: darray<string, varray<mixed>>;
  final public function hasAttribute(string $name)[]: bool;
  final public function getAttribute(string $name)[]: ?varray<mixed>;
}

class ReflectionExtension implements Reflector {

  public $name = '';

  private function __clone();
  public static function export($name, $return = false);
  public function __construct($name)[];
  public function __toString()[];
  public function getName()[];
  public function getVersion()[];
  public function getFunctions()[];
  public function getConstants()[];
  public function getINIEntries()[];
  public function getClasses()[];
  public function getClassNames()[];
  public function info()[];
}

class ReflectionTypeConstant implements Reflector {

  private function __clone();
  public static function export($class, string $name, $return = null);
  public function __construct($class, string $name)[];
  public function __toString()[]: string;
  public function getName()[]: string;
  public function isAbstract()[]: bool;
  public function getDeclaringClass()[]: ReflectionClass;
  public function getClass()[]: ReflectionClass;
  public function getAssignedTypeText()[]: ?string;
  public function getTypeStructure()[]: darray;
}

class ReflectionTypeAlias implements Reflector {
  private function __clone();
  final public function __construct(string $name)[];
  public function __toString()[]: string;
  public function getTypeStructure()[]: darray;
  public function getResolvedTypeStructure()[]: darray;
  public function getAssignedTypeText()[]: string;
  public function getName()[]: string;
  public function getFileName()[]: string;
  public function getFile()[]: ReflectionFile;
  final public function getAttributes()[]: darray<string, varray<mixed>>;
  final public function hasAttribute(string $name)[]: bool;
  final public function getAttribute(string $name)[]: ?varray<mixed>;
  final public function getAttributeClass<T as HH\TypeAliasAttribute>(classname<T> $c)[]: ?T;
}

class ReflectionType {
  private function __clone();
  public function __construct(?Reflector $param_or_ret = null,
                              darray $type_hint_info = darray[]);
  public function allowsNull()[]: bool;
  public function isBuiltin()[]: bool;
  public function __toString()[]: string;
}

class ReflectionFile implements Reflector {
  private function __clone();
  final public function __construct(string $name)[];
  public function __toString()[]: string;
  public function getName()[]: string;
  final public function getAttributes()[]: darray<string, varray<mixed>>;
  final public function hasAttribute(string $name)[]: bool;
  final public function getAttribute(string $name)[]: ?varray<mixed>;
  final public function getAttributeClass<T as HH\FileAttribute>(classname<T> $c)[]: ?T;
}
