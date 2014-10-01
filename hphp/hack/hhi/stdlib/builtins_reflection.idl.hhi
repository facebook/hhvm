<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
function hphp_get_extension_info($name) { }
function hphp_get_class_constant($cls, $name) { }
function hphp_invoke($name, $params) { }
function hphp_invoke_method($obj, $cls, $name, $params) { }
function hphp_instanceof($obj, $name) { }
function hphp_create_object_without_constructor($name) { }
function hphp_get_property($obj, $cls, $prop) { }
function hphp_set_property($obj, $cls, $prop, $value) { }
function hphp_get_static_property($cls, $prop) { }
function hphp_set_static_property($cls, $prop, $value) { }
function hphp_scalar_typehints_enabled() { }
class ReflectionException extends Exception  {
}
class Reflection  {
    public static function getModifierNames($modifiers) {}
    public static function export(Reflector $reflector, $return = false) {}
}
interface Reflector  {
    public function __toString();
}
abstract class ReflectionFunctionAbstract implements Reflector {
    public $name = '';
    final private function __clone() {}
    public function __toString() {}
    public function inNamespace() {}
    public function isClosure() {}
    public function isDeprecated() {}
    public function isInternal() {}
    public function isUserDefined() {}
    public function getClosureThis() {}
    public function getClosureScopeClass() {}
    public function getDocComment() {}
    public function getEndLine() {}
    public function getExtension() {}
    public function getExtensionName() {}
    public function getFileName() {}
    public function getName() {}
    public function getNamespaceName() {}
    public function getNumberOfParameters() {}
    public function getNumberOfRequiredParameters() {}
    public function getParameters() {}
    public function getShortName() {}
    public function getStartLine() {}
    public function getStaticVariables() {}
    public function returnsReference() {}
}
class ReflectionFunction extends ReflectionFunctionAbstract implements Reflector {
    const IS_DEPRECATED = 262144;
    public $name = '';
    public function __construct($name) {}
    public static function export($name, $return = null) {}
    public function isDisabled() {}
    public function invoke($args = null) {}
    public function invokeArgs(array $args) {}
    public function getClosure() {}
}
class ReflectionParameter implements Reflector {
    public $name = '';
    final private function __clone() {}
    public static function export($function, $parameter, $return = null) {}
    public function __construct($function, $parameter) {}
    public function __toString() {}
    public function getName() {}
    public function isPassedByReference() {}
    public function canBePassedByValue() {}
    public function getDeclaringFunction() {}
    public function getDeclaringClass() {}
    public function getClass() {}
    public function isArray() {}
    public function isCallable() {}
    public function allowsNull() {}
    public function getPosition() {}
    public function isOptional() {}
    public function isDefaultValueAvailable() {}
    public function getDefaultValue() {}
    public function isDefaultValueConstant() {}
    public function getDefaultValueConstantName() {}
}
class ReflectionMethod extends ReflectionFunctionAbstract implements Reflector {
    const IS_STATIC = 1;
    const IS_PUBLIC = 256;
    const IS_PROTECTED = 512;
    const IS_PRIVATE = 1024;
    const IS_ABSTRACT = 2;
    const IS_FINAL = 4;
    public $name = '';
    public $class = '';
    public static function export($class, $name, $return = false) {}
    public function __construct($class, $name) {}
    public function isPublic() {}
    public function isPrivate() {}
    public function isProtected() {}
    public function isAbstract() {}
    public function isFinal() {}
    public function isStatic() {}
    public function isConstructor() {}
    public function isDestructor() {}
    public function getClosure($object) {}
    public function getModifiers() {}
    public function invoke($object, $parameter = null, $_ = null) {}
    public function invokeArgs($object, array $args) {}
    public function getDeclaringClass() {}
    public function getPrototype() {}
    public function setAccessible($accessible) {}
}
class ReflectionClass implements Reflector {
    const IS_IMPLICIT_ABSTRACT = 16;
    const IS_EXPLICIT_ABSTRACT = 32;
    const IS_FINAL = 64;
    public $name = '';
    final private function __clone() {}
    public static function export($argument, $return = false) {}
    public function __construct($argument) {}
    public function __toString() {}
    public function getName() {}
    public function isInternal() {}
    public function isUserDefined() {}
    public function isInstantiable() {}
    public function isCloneable() {}
    public function getFileName() {}
    public function getStartLine() {}
    public function getEndLine() {}
    public function getDocComment() {}
    public function getConstructor() {}
    public function hasMethod($name) {}
    public function getMethod($name) {}
    public function getMethods($filter = null) {}
    public function hasProperty($name) {}
    public function getProperty($name) {}
    public function getProperties($filter = null) {}
    public function hasConstant($name) {}
    public function getConstants() {}
    public function getConstant($name) {}
    public function getInterfaces() {}
    public function getInterfaceNames() {}
    public function isInterface() {}
    public function getTraits() {}
    public function getTraitNames() {}
    public function getTraitAliases() {}
    public function isTrait() {}
    public function isAbstract() {}
    public function isFinal() {}
    public function getModifiers() {}
    public function isInstance($object) {}
    public function newInstance($args = null, $_ = null) {}
    public function newInstanceWithoutConstructor() {}
    public function newInstanceArgs(array $args = []) {}
    public function getParentClass() {}
    public function isSubclassOf($class) {}
    public function getStaticProperties() {}
    public function getStaticPropertyValue($name, $default = null) {}
    public function setStaticPropertyValue($name, $value) {}
    public function getDefaultProperties() {}
    public function isIterateable() {}
    public function implementsInterface($interface) {}
    public function getExtension() {}
    public function getExtensionName() {}
    public function inNamespace() {}
    public function getNamespaceName() {}
    public function getShortName() {}
}
class ReflectionObject extends ReflectionClass implements Reflector {
}
class ReflectionProperty implements Reflector {
    const IS_STATIC = 1;
    const IS_PUBLIC = 256;
    const IS_PROTECTED = 512;
    const IS_PRIVATE = 1024;
    public $name = '';
    public $class = '';
    final private function __clone() {}
    public static function export($class, $name, $return = null) {}
    public function __construct($class, $name) {}
    public function __toString() {}
    public function getName() {}
    public function getValue($object) {}
    public function setValue($object, $value) {}
    public function isPublic() {}
    public function isPrivate() {}
    public function isProtected() {}
    public function isStatic() {}
    public function isDefault() {}
    public function getModifiers() {}
    public function getDeclaringClass() {}
    public function getDocComment() {}
    public function setAccessible($accessible) {}
}
class ReflectionExtension implements Reflector {
    public $name = '';
    final private function __clone() {}
    public static function export($name, $return = false) {}
    public function __construct($name) {}
    public function __toString() {}
    public function getName() {}
    public function getVersion() {}
    public function getFunctions() {}
    public function getConstants() {}
    public function getINIEntries() {}
    public function getClasses() {}
    public function getClassNames() {}
    public function getDependencies() {}
    public function info() {}
    public function isPersistent() {}
    public function isTemporary() {}
}
class ReflectionZendExtension implements Reflector {
    public $name = '';
    final private function __clone() {}
    public static function export($name, $return = null) {}
    public function __construct($name) {}
    public function __toString() {}
    public function getName() {}
    public function getVersion() {}
    public function getAuthor() {}
    public function getURL() {}
    public function getCopyright() {}
}
