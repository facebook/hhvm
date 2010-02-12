<?php

///////////////////////////////////////////////////////////////////////////////
// helpers

interface Reflector {
  public function __toString();
  public static function export();
}

class ReflectionException extends Exception {
}

///////////////////////////////////////////////////////////////////////////////
// parameter

class ReflectionParameter implements Reflector {
  public $info;

  public function __construct($func, $param) {
    if ($func && $param) {
      $params = $func->getParameters();
      $this->info = $params[$param]->info;
    }
  }

  public function __toString() {
    // TODO
  }

  public static function export($func, $param, $ret) {
    $obj = new ReflectionParameter($func, $param);
    $str = (string)$obj;
    if ($ret) {
      return $str;
    }
    print $str;
  }

  public function getName() {
    return $this->info['name'];
  }

  public function isPassedByReference() {
    return $this->info['ref'];
  }

  public function getDeclaringClass() {
    if (empty($this->info['class'])) {
      return null;
    }
    return new ReflectionClass($this->info['class']);
  }

  public function getClass() {
    if (empty($this->info['type'])) {
      return null;
    }
    return new ReflectionClass($this->info['type']);
  }

  public function isArray() {
    return $this->info['type'] == 'array';
  }

  public function allowsNull() {
    return $this->info['nullable'];
  }

  public function isOptional() {
    return isset($this->info['default']);
  }

  public function isDefaultValueAvailable() {
    return isset($this->info['default']);
  }

  public function getDefaultValue() {
    if (!$this->isOptional()) {
      throw new ReflectionException('Parameter is not optional');
    }
    return $this->info['default'];
  }

  public function getPosition() {
    return $this->info['index'];
  }
}

///////////////////////////////////////////////////////////////////////////////

class ReflectionFunctionAbstract {
  protected $info;

  public function getName() {
    return $this->info['name'];
  }

  public function isInternal() {
    return $this->info['internal'];
  }

  public function getClosure() {
    return $this->info['closure'];
  }

  public function isUserDefined() {
    return !$this->info['internal'];
  }

  public function getFileName() {
    return $this->info['file'];
  }

  public function getStartLine() {
    return $this->info['line1'];
  }

  public function getEndLine() {
    return $this->info['line2'];
  }

  public function getDocComment() {
    return $this->info['doc'];
  }

  public function getStaticVariables() {
    return $this->info['static_variables'];
  }

  public function returnsReference() {
    return $this->info['ref'];
  }

  public function getParameters() {
    $ret = array();
    foreach ($this->info['params'] as $name => $info) {
      $param = new ReflectionParameter(null, null);
      $param->info = $info;
      $ret[] = $param;
    }
    return $ret;
  }

  public function getNumberOfParameters() {
    return count($this->info['params']);
  }

  public function getNumberOfRequiredParameters() {
    $count = 0;
    $params = $this->getParameters();
    foreach ($params as $name => $param) {
      if ($param->isOptional()) {
        break;
      }
      $count++;
    }
    return $count;
  }
}

///////////////////////////////////////////////////////////////////////////////
// function

class ReflectionFunction extends ReflectionFunctionAbstract
implements Reflector {
  const IS_DEPRECATED = 262144;

  public function __construct($name) {
    $this->info = hphp_get_function_info($name);
    if (empty($this->info)) {
      throw new ReflectionException("Function $name does not exist");
    }
  }

  public function __toString() {
    //TODO
  }

  public static function export($name, $ret) {
    $obj = new ReflectionFunction($name);
    $str = (string)$obj;
    if ($ret) {
      return $str;
    }
    print $str;
  }

  public function invoke() {
    $args = func_get_args();
    return hphp_invoke($this->info['name'], $args);
  }

  public function invokeArgs($args) {
    return hphp_invoke($this->info['name'], $args);
  }
}

///////////////////////////////////////////////////////////////////////////////
// class

class ReflectionClass implements Reflector {
  const IS_IMPLICIT_ABSTRACT = 16 ;
  const IS_EXPLICIT_ABSTRACT = 32 ;
  const IS_FINAL = 64 ;

  public $name;
  protected $info;

  public function __construct($name) {
    $this->info = call_user_func("hphp_get_class_info", $name);
    if (empty($this->info)) {
      if (is_object($name)) $name = get_class($name);
      throw new ReflectionException("Class $name does not exist");
    }
    $this->name = $this->info['name'];

    // flattening the trees, so it's easier for lookups
    if (!empty($this->info['parent'])) {
      $p = new ReflectionClass($this->info['parent']);
      $this->info['interfaces'] += $p->info['interfaces'];
      $this->info['properties'] += $p->info['properties'];
      $this->info['methods']    += $p->info['methods'];
      $this->info['constants']  += $p->info['constants'];
    }
    foreach ($this->info['interfaces'] as $interface => $_) {
      $p = new ReflectionClass($interface);
      $this->info['methods'] += $p->info['methods'];
    }
  }

  public function __toString() {
  }

  public static function export($name, $ret) {
    $obj = new ReflectionClass($name);
    $str = (string)$obj;
    if ($ret) {
      return $str;
    }
    print $str;
  }

  public function getName() {
    return $this->info['name'];
  }

  public function isInternal() {
    return $this->info['internal'];
  }

  public function isUserDefined() {
    return !$this->info['internal'];
  }

  public function isInstantiable() {
    return !$this->info['abstract'];
  }

  public function hasConstant($name) {
    return isset($this->info['constants'][$name]);
  }

  public function hasMethod($name) {
    return isset($this->info['methods'][$name]);
  }

  public function hasProperty($name) {
    return isset($this->info['properties'][$name]);
  }

  public function getFileName() {
    return $this->info['file'];
  }

  public function getStartLine() {
    return $this->info['line1'];
  }

  public function getEndLine() {
    return $this->info['line2'];
  }

  public function getDocComment() {
    return $this->info['doc'];
  }

  public function getConstructor() {
    if ($this->hasMethod('__construct')) {
      return $this->getMethod('__construct');
    }
    if ($this->hasMethod($this->info['name'])) {
      return $this->getMethod($this->info['name']);
    }
    return null;
  }

  public function getMethod($name) {
    $lname = strtolower($name);
    if (!isset($this->info['methods'][$lname])) {
      $class = $this->info['name'];
      throw new ReflectionException("Method $class::$name does not exist");
    }

    $ret = new ReflectionMethod(null, null);
    $ret->info  = $this->info['methods'][$lname];
    $ret->name  = $lname;
    $ret->class = $this->info['name'];
    return $ret;
  }

  public function getMethods() {
    $ret = array();
    foreach ($this->info['methods'] as $name => $_) {
      $ret[] = $this->getMethod($name);
    }
    return $ret;
  }

  public function getProperty($name) {
    if (!isset($this->info['properties'][$name])) {
      $class = $this->info['name'];
      throw new ReflectionException("Property $class::$name does not exist");
    }

    $ret = new ReflectionProperty(null, null);
    $ret->info  = $this->info['properties'][$name];
    $ret->name  = $name;
    $ret->class = $this->info['name'];
    return $ret;
  }

  public function getProperties() {
    $ret = array();
    foreach ($this->info['properties'] as $name => $_) {
      $ret[] = $this->getProperty($name);
    }
    return $ret;
  }

  public function getConstants() {
    return $this->info['constants'];
  }

  public function getConstant($name) {
    if (!isset($this->info['constants'][$name])) {
      $class = $this->info['name'];
      throw new ReflectionException("Class constant $class::$name does not exist");
    }
    return $this->info['constants'][$name];
  }

  public function getInterfaces() {
    $ret = array();
    foreach ($this->info['interfaces'] as $name => $_) {
      $cls = new ReflectionClass($name);
      if ($cls->isInterface()) {
        $ret[$name] = $cls;
      }
    }
    return $ret;
  }

  public function isInterface() {
    return $this->info['interface'];
  }

  public function isAbstract() {
    return $this->info['abstract'];
  }

  public function isFinal() {
    return $this->info['final'];
  }

  public function getModifiers() {
    return $this->info['modifiers'];
  }

  public function isInstance($obj) {
    return hphp_instanceof($obj, $this->info['name']);
  }

  public function newInstance() {
    $args = func_get_args();
    return hphp_create_object($this->info['name'], $args);
  }

  public function newInstanceArgs($args) {
    return hphp_create_object($this->info['name'], $args);
  }

  public function getParentClass() {
    if (empty($this->info['parent'])) {
      return false;
    }
    return new ReflectionClass($this->info['parent']);
  }

  public function isSubclassOf($cls) {
    if ($cls instanceof ReflectionClass) {
      $cls = $cls->info['name'];
    }
    foreach ($this->info['interfaces'] as $name => $_) {
      if (strcasecmp($cls, $name) == 0) {
        return true;
      }
    }
    if (empty($this->info['parent'])) {
      return false;
    }
    if (strcasecmp($cls, $this->info['parent']) == 0) {
      return true;
    }
    return $this->getParentClass()->isSubclassOf($cls);
  }

  public function getStaticProperties() {
    $ret = array();
    foreach ($this->getProperties() as $prop) {
      if ($prop->isStatic()) {
        $ret[$prop->name] = $prop;
      }
    }
    return $ret;
  }

  public function getStaticPropertyValue($name, $default = null) {
    if ($this->hasProperty($name) &&
        $this->getProperty($name)->isStatic()) {
      return hphp_get_static_property($this->info['name'], $name);
    }
    return $default;
  }

  public function setStaticPropertyValue($name, $value) {
    hphp_set_static_property($this->info['name'], $name, $value);
  }

  public function getDefaultProperties() {
    $ret = array();
    foreach ($this->getProperties() as $prop) {
      if ($prop->isDefault()) {
        $ret[$prop->name] = $prop;
      }
    }
    return $ret;
  }

  public function isIterateable() {
    return $this->isSubclassOf('ArrayAccess');
  }

  public function implementsInterface($cls) {
    if ($cls instanceof ReflectionClass) {
      $cls = $cls->info['name'];
    }
    foreach ($this->info['interfaces'] as $name => $_) {
      if (strcasecmp($cls, $name) == 0) {
        return true;
      }
    }
    return false;
  }

  public function getExtension() {
    return $this->info['extension'];
  }

  public function getExtensionName() {
    return $this->info['extension']->getName();
  }
}

///////////////////////////////////////////////////////////////////////////////
// object

class ReflectionObject extends ReflectionClass {
  public function __construct($obj) {
    $this->info = hphp_get_class_info($obj);
  }

  public static function export($obj, $ret) {
    $obj = new ReflectionObject($obj);
    $str = (string)$obj;
    if ($ret) {
      return $str;
    }
    print $str;
  }
}

///////////////////////////////////////////////////////////////////////////////
// property

class ReflectionProperty implements Reflector {
  const IS_STATIC = 1;
  const IS_PUBLIC = 256;
  const IS_PROTECTED = 512;
  const IS_PRIVATE = 1024;

  public $info;
  public $name;
  public $class;

  public function __construct($cls, $name) {
    if ($cls && $name) {
      if (!is_object($cls)) $cls = new ReflectionClass($cls);
      $prop = $cls->getProperty($name);
      if ($prop) {
        $this->info  = $prop->info;
        $this->name  = $prop->name;
        $this->class = $prop->class;
      }
    }
  }

  public function __toString() {
  }

  public static function export($cls, $name, $ret) {
    if (!is_object($cls)) $cls = new ReflectionClass($cls);
    $obj = $cls->getProperty($name);
    $str = (string)$obj;
    if ($ret) {
      return $str;
    }
    print $str;
  }

  public function getName() {
    return $this->info['name'];
  }

  public function isPublic() {
    return $this->info['access'] == 'public';
  }

  public function isPrivate() {
    return $this->info['access'] == 'private';
  }

  public function isProtected() {
    return $this->info['access'] == 'protected';
  }

  public function isStatic() {
    return $this->info['static'];
  }

  public function isDefault() {
    return $this->info['default'];
  }

  public function setAccessible() {
  }

  public function getModifiers() {
    return $this->info['modifiers'];
  }

  public function getValue($obj) {
    if ($this->isStatic()) {
      return hphp_get_static_property($this->info['class'],
                                      $this->info['name']);
    }
    return hphp_get_property($obj, $this->info['class'], $this->info['name']);
  }

  public function setValue($obj, $value) {
    if ($this->isStatic()) {
      return hphp_set_static_property($this->info['class'],
                                      $this->info['name'], $value);
    }
    hphp_set_property($obj, $this->info['class'], $this->info['name'], $value);
  }

  public function getDeclaringClass() {
    if (empty($this->info['class'])) {
      return null;
    }
    return new ReflectionClass($this->info['class']);
  }

  public function getDocComment() {
    return $this->info['doc'];
  }
}

///////////////////////////////////////////////////////////////////////////////
// method

class ReflectionMethod extends ReflectionFunctionAbstract
implements Reflector {
  const IS_STATIC = 1;
  const IS_PUBLIC = 256;
  const IS_PROTECTED = 512;
  const IS_PRIVATE = 1024;
  const IS_ABSTRACT = 2;
  const IS_FINAL = 4;

  public $name;
  public $class;

  public function __construct($cls, $name) {
    if ($cls && $name) {
      if (!is_object($cls)) $cls = new ReflectionClass($cls);
      $method = $cls->getMethod($name);
      if ($method) {
        $this->info  = $method->info;
        $this->name  = $method->name;
        $this->class = $method->class;
      }
    }
  }

  public function __toString() {
    //TODO
  }

  public static function export($cls, $name, $ret) {
    if (!is_object($cls)) $cls = new ReflectionClass($cls);
    $obj = $cls->getMethod($name);
    $str = (string)$obj;
    if ($ret) {
      return $str;
    }
    print $str;
  }

  public function invoke($obj) {
    $args = func_get_args();
    array_shift($args);
    return hphp_invoke_method($obj, $this->info['class'], $this->info['name'],
                              $args);
  }

  public function invokeArgs($obj, $args) {
    return hphp_invoke_method($obj, $this->info['class'], $this->info['name'],
                              $args);
  }

  public function isFinal() {
    return $this->info['final'];
  }

  public function isAbstract() {
    return $this->info['abstract'];
  }

  public function isPublic() {
    return $this->info['access'] == "public";
  }

  public function isPrivate() {
    return $this->info['access'] == "private";
  }

  public function isProtected() {
    return $this->info['access'] == "protected";
  }

  public function isStatic() {
    return $this->info['static'];
  }

  public function isConstructor() {
    return $this->getName() == '__construct';
  }

  public function isDestructor() {
    return $this->getName() == '__destruct';
  }

  public function getModifiers() {
    return $this->info['modifiers'];
  }

  public function getClosure() {
    return $this->info['closure'];
  }

  public function getDeclaringClass() {
    if (empty($this->info['class'])) {
      return null;
    }
    return new ReflectionClass($this->info['class']);
  }
}

///////////////////////////////////////////////////////////////////////////////
// extension

class ReflectionExtension implements Reflector {
  private $name;
  private $info;

  public function __construct($name) {
    $this->info = hphp_get_extension_info($name);
  }

  public function __toString() {
  }

  public static function export($name, $ret) {
    $obj = new ReflectionExtension($name);
    $str = (string)$obj;
    if ($ret) {
      return $str;
    }
    print $str;
  }

  public function getName() {
    return $this->info['name'];
  }

  public function getVersion() {
    return $this->info['version'];
  }

  public function getFunctions() {
    return $this->info['functions'];
  }

  public function getConstants() {
    return $this->info['constants'];
  }

  public function getINIEntries() {
    return $this->info['ini'];
  }

  public function getClasses() {
    return $this->info['classes'];
  }

  public function getClassNames() {
    $ret = array();
    foreach ($this->info['classes'] as $cls) {
      $ret[] = $cls->getName();
    }
    return $ret;
  }

  public function info() {
    return $this->info['info'];
  }
}
