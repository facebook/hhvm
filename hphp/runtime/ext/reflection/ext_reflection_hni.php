<?hh

/**
 * ( excerpt from
 * http://php.net/manual/en/class.reflectionfunctionabstract.php )
 *
 * A parent class to ReflectionFunction and ParentMethod. Read their
 * descriptions for details.
 */
<<__NativeData('ReflectionFuncHandle')>>
abstract class ReflectionFunctionAbstract implements Reflector {

  const IS_STATIC    = 1;
  const IS_PUBLIC    = 256;
  const IS_PROTECTED = 512;
  const IS_PRIVATE   = 1024;
  const IS_ABSTRACT  = 2;
  const IS_FINAL     = 4;

  /**
   * (excerpt from
   * http://php.net/manual/en/reflectionfunctionabstract.getname.php )
   *
   * Get the name of the function. Warning: This function is currently not
   * documented; only its argument list is available.
   *
   * @return     string   The name of the function.
   */
  <<__Native>>
  public function getName(): string;

  /**
   * ( excerpt from http://php.net/manual/en/reflectionfunctionabstract.innamespace.php
   * )
   *
   * Checks whether a function is defined in a namespace.
   *
   * @return     bool   Returns TRUE on success or FALSE on failure.
   */
  final public function inNamespace(): bool {
    return strrpos($this->getName(), '\\') !== false;
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionfunctionabstract.getnamespacename.php )
   *
   * Get the namespace name where the function is defined.
   *
   * @return     string   The namespace name.
   */
  final public function getNamespaceName(): string {
    $name = $this->getName();
    $pos = strrpos($name, '\\');
    return ($pos === false) ? '' : substr($name, 0, $pos);
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionfunctionabstract.getshortname.php )
   *
   * Get the short name of the function (without the namespace part).
   *
   * @return     string  The short name of the function.
   */
  final public function getShortName(): string {
    $name = $this->getName();
    $pos = strrpos($name, '\\');
    return ($pos === false) ? $name : substr($name, $pos + 1);
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionfunctionabstract.isinternal.php )
   *
   * Checks whether the function is internal, as opposed to user-defined.
   * Warning: This function is currently not documented; only its argument
   * list is available.
   *
   * @return     mixed   TRUE if it's internal, otherwise FALSE
   */
  <<__Native>>
  public function isInternal(): bool;

  abstract public function getClosure();

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionfunctionabstract.isclosure.php )
   *
   * Checks whether it's a closure. Warning: This function is currently not
   * documented; only its argument list is available.
   *
   * @return     bool   TRUE if it's a closure, otherwise FALSE
   */
  abstract public function isClosure(): bool;

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionfunctionabstract.isgenerator.php )
   *
   * Warning: This function is currently not documented; only its argument
   * list is available.
   *
   * @return     bool   TRUE if the function is generator, otherwise FALSE.
   */
  <<__Native>>
  public function isGenerator(): bool;

  /**
   * @return     bool   TRUE if the function is async, otherwise FALSE.
   */
  <<__Native, __HipHopSpecific>>
  public function isAsync(): bool;

  /**
   * Indicates whether the function has ...$varargs as its last parameter
   * to capture variadic arguments.
   *
   * @return     bool   TRUE if the function is variadic, otherwise FALSE
   */
  <<__Native>>
  public function isVariadic(): bool;

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionfunctionabstract.isuserdefined.php )
   *
   * Checks whether the function is user-defined, as opposed to internal.
   * Warning: This function is currently not documented; only its argument
   * list is available.
   *
   * @return     mixed   TRUE if it's user-defined, otherwise false;
   */
  public function isUserDefined(): bool {
    return !$this->isInternal();
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionfunctionabstract.getfilename.php )
   *
   * Gets the file name from a user-defined function. Warning: This function
   * is currently not documented; only its argument list is available.
   *
   * @return     mixed   The file name.
   */
  <<__Native>>
  public function getFileName(): string;

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionfunctionabstract.getstartline.php )
   *
   * Gets the starting line number of the function. Warning: This function
   * is currently not documented; only its argument list is available.
   *
   * @return     mixed   The starting line number.
   */
  <<__Native>>
  public function getStartLine(): int;

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionfunctionabstract.getendline.php )
   *
   * Get the ending line number. Warning: This function is currently not
   * documented; only its argument list is available.
   *
   * @return     mixed   The ending line number of the user defined function,
   *                     or FALSE if unknown.
   */
  <<__Native>>
  public function getEndLine(): int;

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionfunctionabstract.getdoccomment.php )
   *
   * Get a Doc comment from a function. Warning: This function is currently
   * not documented; only its argument list is available.
   *
   * @return     mixed   The doc comment string if it exists, otherwise FALSE
   */
  <<__Native>>
  public function getDocComment(): mixed;

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionfunctionabstract.getstaticvariables.php
   * )
   *
   * Get the static variables. Warning: This function is currently not
   * documented; only its argument list is available.
   *
   * @return     array<string, mixed>   An array of static variables.
   */
  <<__Native>>
  public function getStaticVariables(): array;

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionfunctionabstract.returnsreference.php
   * )
   *
   * Checks whether the function returns a reference. Warning: This function
   * is currently not documented; only its argument list is available.
   *
   * @return     mixed   TRUE if it returns a reference, otherwise FALSE
   */
  <<__Native>>
  public function returnsReference(): bool;

  private static function stripHHPrefix($str) {
    if (!is_string($str)) return $str;
    return str_ireplace(
      array('HH\\bool', 'HH\\int', 'HH\\float', 'HH\\string', 'HH\\num',
            'HH\\resource', 'HH\\void', 'HH\\this'),
      array('bool',     'int',     'float',     'string',     'num',
            'resource',     'void',    'this'),
      $str
    );
  }

  <<__Native, __HipHopSpecific>>
  private function getReturnTypeHint(): string;

  <<__HipHopSpecific>>
  public function getReturnTypeText() {
    $hint = $this->getReturnTypeHint();
    return ($hint) ? self::stripHHPrefix($hint) : false;
  }

  <<__Native>>
  final public function getAttributes(): array;

  final public function getAttribute(string $name) {
    return hphp_array_idx($this->getAttributes(), $name, null);
  }

  abstract public function getAttributesRecursive(): array;

  abstract public function getAttributeRecursive(string $name);

  <<__Native>>
  public function getNumberOfParameters(): int;

  <<__Native>>
  private function getParamInfo(): array;

  private $params = null;

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionfunctionabstract.getparameters.php )
   *
   * Get the parameters as an array of ReflectionParameter. Warning: This
   * function is currently not documented; only its argument list is
   * available.
   *
   * @return     array  The parameters, as a ReflectionParameter object.
   */
  public function getParameters(): array<ReflectionParameter> {
    // FIXME: ReflectionParameter sh/could have native data pointing to the
    // relevant Func::ParamInfo data structure
    if (null === $this->params) {
      $ret = array();
      foreach ($this->getParamInfo() as $name => $info) {
        $param = new ReflectionParameter(null, null);
        $param->info = $info;
        $param->name = $info['name'];
        $ret[] = $param;
      }
      $this->params = $ret;
    }
    return $this->params;
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionfunctionabstract.getnumberofrequiredparameters.php
   * )
   *
   * Get the number of required parameters that a function defines. Warning:
   * This function is currently not documented; only its argument list is
   * available.
   *
   * @return     mixed   The number of required parameters.
   */
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

/**
 * ( excerpt from http://php.net/manual/en/class.reflectionfunction.php )
 *
 * The ReflectionFunction class reports information about a function.
 */
class ReflectionFunction extends ReflectionFunctionAbstract {

  /* public readonly string $name; */
  private ?Closure $closure = null;

  /**
   * ( excerpt from http://php.net/manual/en/reflectionfunction.construct.php
   * )
   *
   * Constructs a ReflectionFunction object.
   *
   * @name       mixed   The name of the function to reflect or a closure.
   *
   * @return     mixed   No value is returned.
   */
  public function __construct($name_or_closure) {
    if ($name_or_closure instanceof Closure) {
      $this->closure = $name_or_closure;
      $this->__initClosure($name_or_closure);
    } else if (is_string($name_or_closure)){
      if (!$this->__initName($name_or_closure)) {
        throw new ReflectionException(
          "Function $name_or_closure does not exist");
      }
    } else {
      throw new ReflectionException(
        sprintf('%s expects a string or a Closure, got %s',
                __METHOD__, gettype($name_or_closure))
      );
    }
  }

  <<__Native>>
  private function __initClosure(object $closure): void;

  <<__Native>>
  private function __initName(string $name): bool;

  /**
   * (excerpt from
   * http://php.net/manual/en/reflectionfunctionabstract.getname.php )
   *
   * Get the name of the function. Warning: This function is currently not
   * documented; only its argument list is available.
   *
   * @return     string   The name of the function.
   */
  public function getName(): string {
    if ($this->closure) {
      $clsname = $this->getClosureScopeClassname($this->closure);
      $pos = $clsname ? strrpos($clsname, '\\') : false;
      return ($pos === false)
        ? '{closure}'
        : substr($clsname, 0, $pos + 1).'{closure}';
    }
    return parent::getName();
  }

  // __get and __set are used to maintain read-only $this->name
  final public function __get(string $name): ?string {
    // $name is a read-only property
    if ($name === 'name') {
      return $this->getName();
    }
    $static_cls = get_class($this);
    if (property_exists($static_cls, $name)) {
      // __get is called if an existing property is inaccessible
      trigger_error("Cannot access property $static_cls::$name", E_USER_ERROR);
    }
    trigger_error("Undefined property $static_cls::$name", E_USER_NOTICE);
    return null;
  }

  // __get and __set are used to maintain read-only $this->name
  final public function __set(string $name, $value): void {
    // $name is a read-only property
    if ($name === 'name') {
      throw new ReflectionException(
        'Cannot set read-only property '.__CLASS__.'::'.$name);
    }
    if (property_exists(get_class($this), $name)) {
      // __set is called if the property is inaccessible
      trigger_error(
        'Cannot access property '.get_class($this).'::'.$name, E_USER_ERROR);
    }
    $this->{$name} = $value;
  }

  public function getClosure() {
    return $this->closure ?:
      function(...$args) { return $this->invokeArgs($args); };
  }

  public function isClosure(): bool {
    return (bool) $this->closure;
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionmethod.tostring.php )
   *
   * Returns the string representation of the Reflection method object.
   *
   * @return     mixed   A string representation of this ReflectionMethod
   *                     instance.
   */
  public function __toString(): string {
    // TODO
    return '';
  }

  // Prevent cloning
  public function __clone() {
    throw new BadMethodCallException(
      'Trying to clone an uncloneable object of class ' . get_class($this),
    );
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionfunction.export.php )
   *
   * Exports a Reflected function.
   *
   * @name       mixed   The reflection to export.
   * @ret        mixed   Setting to TRUE will return the export, as opposed
   *                     to emitting it. Setting to FALSE (the default) will
   *                     do the opposite.
   *
   * @return     mixed   If the return parameter is set to TRUE, then the
   *                     export is returned as a string, otherwise NULL is
   *                     returned.
   */
  public static function export($name, $ret=false) {
    $obj = new self($name);
    $str = (string) $obj;
    if ($ret) {
      return $str;
    }
    print $str;
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionfunctionabstract.getstaticvariables.php
   * )
   *
   * Get the static variables. Warning: This function is currently not
   * documented; only its argument list is available.
   *
   * @return     mixed   An array of static variables.
   */
  public function getStaticVariables() {
    $static_vars = parent::getStaticVariables();
    return !$this->closure
      ? $static_vars
      : array_merge( // XXX: which should win in key collision case?
        $static_vars,
        $this->getClosureUseVariables($this->closure),
      );
  }

  <<__Native>>
  private function getClosureUseVariables(object $closure): array;

  /**
   * ( excerpt from http://php.net/manual/en/reflectionfunction.invoke.php )
   *
   * Invokes a reflected function.
   *
   * @return     mixed   Returns the result of the invoked function call.
   */
  public function invoke(...$args) {
    if ($this->closure) {
      return hphp_invoke_method($this->closure, get_class($this->closure),
                                '__invoke', $args);
    }
    return hphp_invoke($this->getName(), $args);
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionfunction.invokeargs.php )
   *
   * Invokes the function and pass its arguments as array.
   *
   * @args       mixed   The passed arguments to the function as an array,
   *                     much like call_user_func_array() works.
   *
   * @return     mixed   Returns the result of the invoked function
   */
  public function invokeArgs($args) {
    if ($this->closure) {
      return hphp_invoke_method($this->closure, get_class($this->closure),
                                '__invoke', array_values($args));
    }
    return hphp_invoke($this->getName(), array_values($args));
  }

  <<__Native>>
  private function getClosureScopeClassname(object $closure): string;

  public function getClosureScopeClass(): ?ReflectionClass {
    if ($this->closure &&
        ($cls = $this->getClosureScopeClassname($this->closure))) {
      return new ReflectionClass($cls);
    }
    return null;
  }

  public function getAttributesRecursive(): array {
    return $this->getAttributes();
  }

  public function getAttributeRecursive(string $name) {
    return $this->getAttribute($name);
  }
}

/**
 * ( excerpt from http://php.net/manual/en/class.reflectionmethod.php )
 *
 * The ReflectionMethod class reports information about a method.
 */
class ReflectionMethod extends ReflectionFunctionAbstract {

  /* public readonly string $name; */
  /* public readonly string $class; */

  private /*string*/ $originalClass;
  private /*bool*/ $forcedAccessible = false;

  <<__Native>>
  private function __init(mixed $cls_or_obj, string $meth): bool;

  /**
   * ( excerpt from http://php.net/manual/en/reflectionmethod.construct.php )
   *
   * Constructs a new ReflectionMethod.
   *
   * @cls        mixed   Classname or object (instance of the class) that
   *                     contains the method.
   * @name       mixed   Name of the method.
   */
  public function __construct(mixed $cls, string $name = '') {
    if (!$name && is_string($cls)) {
      $arr = explode('::', $cls, 3);
      if (count($arr) !== 2) {
        throw new ReflectionException("$cls is not a valid method name");
      }
      list($cls, $name) = $arr;
      $classname = $cls;
    } else {
      $classname = is_object($cls) ? get_class($cls) : $cls;
    }

    $this->originalClass = $classname;
    if (!$this->__init($cls, $name)) {
      throw new ReflectionException("Method $classname::$name does not exist");
    }
  }

  // __get and __set are used to maintain read-only $this->name, $this->class
  final public function __get(string $name): ?string {
    if ($name === 'name') { // $name is a read-only property
      return $this->getName();
    } else if ($name === 'class') { // ... as is $class
      return $this->getDeclaringClassname();
    }
    $static_cls = get_class($this);
    if (property_exists($static_cls, $name)) {
      // __get is called if an existing property is inaccessible
      trigger_error("Cannot access property $static_cls::$name", E_USER_ERROR);
    }
    trigger_error("Undefined property $static_cls::$name", E_USER_NOTICE);
    return null;
  }

  // __get and __set are used to maintain read-only $this->name, $this->class
  final public function __set(string $name, $value): void {
    // $name and $class are read-only properties
    if ($name === 'name' || $name === 'class') {
      throw new ReflectionException(
        'Cannot set read-only property '.__CLASS__.'::'.$name);
    }
    if (property_exists(get_class($this), $name)) {
      // __set is called if the property is inaccessible
      trigger_error(
        'Cannot access property '.get_class($this).'::'.$name, E_USER_ERROR);
    }
    $this->{$name} = $value;
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionmethod.tostring.php )
   *
   * Returns the string representation of the Reflection method object.
   *
   * @return     mixed   A string representation of this ReflectionMethod
   *                     instance.
   */
  public function __toString(): string {
    // TODO
    return '';
  }

  // Prevent cloning
  public function __clone() {
    throw new BadMethodCallException(
      'Trying to clone an uncloneable object of class ReflectionMethod'
    );
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionmethod.export.php )
   *
   * Exports a ReflectionMethod. Warning: This function is currently not
   * documented; only its argument list is available.
   *
   * @cls        mixed   The class name.
   * @name       mixed   The name of the method.
   * @ret        mixed   Setting to TRUE will return the export, as opposed
   *                     to emitting it. Setting to FALSE (the default) will
   *                     do the opposite.
   *
   * @return     ?string If the return parameter is set to TRUE, then the
   *                     export is returned as a string, otherwise NULL is
   *                     returned.
   */
  public static function export(
    string $cls,
    string $name,
    bool $ret = false,
  ): ?string {
    $meth = new self($cls, $name);
    $str = (string) $meth;
    if ($ret) {
      return $str;
    }
    print $str;
  }

  private function isAccessible(): bool {
    return $this->forcedAccessible || $this->isPublic();
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionmethod.invoke.php )
   *
   * Invokes a reflected method.
   *
   * @obj        mixed   The object to invoke the method on. For static
   *                     methods, pass null to this parameter.
   *
   * @return     mixed   Returns the method result.
   */
  public function invoke($obj, ...$args): mixed {
    if (!$this->isAccessible()) {
      throw new ReflectionException(
        sprintf(
          'Trying to invoke %s method %s::%s() from scope ReflectionMethod',
          ($this->isProtected() ? 'protected' : 'private'),
          $this->getDeclaringClassname(), $this->getName(),
        )
      );
    }
    if ($this->isStatic()) {
      // Docs says to pass null, but Zend completely ignores the argument
      $obj = null;
    }
    return hphp_invoke_method($obj, $this->originalClass,
                              $this->getName(), $args);
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionmethod.invokeargs.php
   * )
   *
   * Invokes the reflected method and pass its arguments as array.
   *
   * @obj        mixed   The object to invoke the method on. In case of
   *                     static methods, you can pass null to this parameter.
   * @args       mixed   The parameters to be passed to the function, as an
   *                     array.
   *
   * @return     mixed   Returns the method result.
   */
  public function invokeArgs($obj, $args): mixed {
    // XXX: is array_values necessary here?
    if ($this->isStatic()) {
      $obj = null;
    }
    return hphp_invoke_method($obj, $this->originalClass, $this->getName(),
                              array_values($args));
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionmethod.isfinal.php )
   *
   * Checks if the method is final.
   *
   * @return     bool   TRUE if the method is final, otherwise FALSE
   */
  <<__Native>>
  public function isFinal(): bool;

  /**
   * ( excerpt from http://php.net/manual/en/reflectionmethod.isabstract.php
   * )
   *
   * Checks if the method is abstract.
   *
   * @return     bool   TRUE if the method is abstract, otherwise FALSE
   */
  <<__Native>>
  public function isAbstract(): bool;

  /**
   * ( excerpt from http://php.net/manual/en/reflectionmethod.ispublic.php )
   *
   * Checks if the method is public.
   *
   * @return     bool   TRUE if the method is public, otherwise FALSE
   */
  <<__Native>>
  public function isPublic(): bool;

  /**
   * ( excerpt from http://php.net/manual/en/reflectionmethod.isprotected.php
   * )
   *
   * Checks if the method is protected.
   *
   * @return     bool   TRUE if the method is protected, otherwise FALSE
   */
  <<__Native>>
  public function isProtected(): bool;

  /**
   * ( excerpt from http://php.net/manual/en/reflectionmethod.isprivate.php )
   *
   * Checks if the method is private. Warning: This function is currently
   * not documented; only its argument list is available.
   *
   * @return     bool   TRUE if the method is private, otherwise FALSE
   */
  <<__Native>>
  public function isPrivate(): bool;

  /**
   * ( excerpt from http://php.net/manual/en/reflectionmethod.isstatic.php )
   *
   * Checks if the method is static.
   *
   * @return     bool   TRUE if the method is static, otherwise FALSE
   */
  <<__Native>>
  public function isStatic(): bool;

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionmethod.isconstructor.php )
   *
   * Checks if the method is a constructor.
   *
   * @return     bool   TRUE if the method is a constructor, otherwise FALSE
   */
  <<__Native>>
  public function isConstructor(): bool;

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionmethod.isdestructor.php )
   *
   * Checks if the method is a destructor.
   *
   * @return     bool   TRUE if the method is a destructor, otherwise FALSE
   */
  public function isDestructor(): bool {
    return $this->getName() == '__destruct';
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionmethod.getmodifiers.php )
   *
   * Returns a bitfield of the access modifiers for this method.
   *
   * @return     int   A numeric representation of the modifiers. The
   *                   modifiers are listed below. The actual meanings of
   *                   these modifiers are described in the predefined
   *                   constants.
   */
  <<__Native>>
  public function getModifiers(): int;

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionmethod.getprototype.php )
   *
   * Returns the methods prototype.
   *
   * @return     object   A ReflectionMethod instance of the method prototype.
   */
  public function getPrototype(): ReflectionMethod {
    $proto_cls = $this->getPrototypeClassname();
    $name = $this->getName();
    if ($proto_cls) {
      return new ReflectionMethod($proto_cls, $name);
    }
    // This message is arguably misleading without $this->class
    throw new ReflectionException(
      sprintf('Method %s::%s does not have a prototype',
              $this->originalClass,
              $name,)
    );
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionmethod.getclosure.php
   * )
   *
   * Warning: This function is currently not documented; only its argument
   * list is available.
   *
   * @object    object  Forbidden for static methods, required for other methods
   *
   * @return    mixed   Returns Closure. Returns NULL in case of an error.
   */
  public function getClosure($object = null): ?Closure {
    if ($this->isStatic()) {
      $object = null;
    } else {
      if (!$object) {
        trigger_error(
          'ReflectionMethod::getClosure() expects parameter 1'
          . ' to be object, ' . gettype($object) . ' given', E_USER_WARNING);
        return null;
      }
      $cls_name = $this->getDeclaringClassname();
      if (!($object instanceof $cls_name)) {
        throw new ReflectionException(
          'Given object is not an instance of the class this method was '.
          'declared in' // mention declaringClassname / originalClass here ?
        );
      }
    }
    return function (...$args) use ($object) {
      return $this->invokeArgs($object, $args);
    };
  }

  public function isClosure(): bool {
    return false;
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionmethod.setaccessible.php )
   *
   * Sets a method to be accessible. For example, it may allow protected and
   * private methods to be invoked.
   *
   * @accessible mixed   TRUE to allow accessibility, or FALSE.
   *
   * @return     mixed   No value is returned.
   */
  public function setAccessible(bool $accessible): void {
    // Public methods are always accessible. Cannot manually set to not be
    // accessible.
    if ($this->isPublic()) { return; }
    $this->forcedAccessible = $accessible;
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionmethod.getdeclaringclass.php )
   *
   * Gets the declaring class for the reflected method.
   *
   * @return ReflectionClass   A ReflectionClass object of the class that the
   *                           reflected method is part of.
   */
  public function getDeclaringClass() {
    return new ReflectionClass($this->getDeclaringClassname());
  }

  <<__Native>>
  private function getDeclaringClassname(): string;

  <<__Native>>
  private function getPrototypeClassname(): string; // ?string

  public function getAttributeRecursive(string $name) {
    $attrs = $this->getAttributes();
    if (isset($attrs[$name])) {
      return $attrs[$name];
    }
    $p = get_parent_class($this->getDeclaringClassname());
    if ($p === false) {
      return null;
    }
    $rm = new ReflectionMethod($p, $this->getName());
    if ($rm->isPrivate()) {
      return null;
    }
    return $rm->getAttributeRecursive($name);
  }

  public function getAttributesRecursive(): array {
    $attrs = $this->getAttributes();
    $p = get_parent_class($this->getDeclaringClassname());
    if ($p !== false) {
      $rm = new ReflectionMethod($p, $this->getName());
      if (!$rm->isPrivate()) {
        $attrs += $rm->getAttributesRecursive();
      }
    }
    return $attrs;
  }
}

/**
 * ( excerpt from http://php.net/manual/en/class.reflectionclass.php )
 *
 * The ReflectionClass class reports information about a class.
 */
<<__NativeData('ReflectionClassHandle')>>
class ReflectionClass implements Reflector {
  const int IS_IMPLICIT_ABSTRACT = 16;
  const int IS_EXPLICIT_ABSTRACT = 32;
  const int IS_FINAL = 64;

  /* public readonly string $name; */
  private $obj = null;

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.construct.php )
   *
   * Constructs a new ReflectionClass object. Warning: This function is
   * currently not documented; only its argument list is available.
   *
   * @name       mixed   Either a string containing the name of the class to
   *                     reflect, or an object.
   */
  public function __construct(mixed $name_or_obj) {
    if (is_object($name_or_obj)) {
      $this->obj = $name_or_obj;
      $classname = get_class($name_or_obj);
    } else {
      $classname = $name_or_obj;
    }
    if (!$this->__init($classname)) {
      throw new ReflectionException("Class $classname does not exist");
    }
  }

  <<__Native>>
  private function __init(string $name): string;

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.tostring.php )
   *
   * Returns the string representation of the ReflectionClass object.
   *
   * @return     string  A string representation of this ReflectionClass
   *                     instance.
   */
  public function __toString(): string {
    // TODO
    return '';
  }

  // Prevent cloning
  public function __clone() {
    throw new BadMethodCallException(
      'Trying to clone an uncloneable object of class ReflectionClass'
    );
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.export.php )
   *
   * Exports a reflected class.
   *
   * @name       mixed   The reflection to export.
   * @ret        mixed   Setting to TRUE will return the export, as opposed
   *                     to emitting it. Setting to FALSE (the default) will
   *                     do the opposite.
   *
   * @return     mixed   If the return parameter is set to TRUE, then the
   *                     export is returned as a string, otherwise NULL is
   *                     returned.
   */
  public static function export($name, bool $ret=false): ?string {
    $obj = new ReflectionClass($name);
    $str = (string) $obj;
    if ($ret) {
      return $str;
    }
    print $str;
  }

  <<__Native>>
  public function getName(): string;

  // __get and __set are used to maintain read-only $this->name
  final public function __get(string $name): ?string {
    // $name is a read-only property
    if ($name === 'name') {
      return $this->getName();
    }
    $static_cls = get_class($this);
    if (property_exists($static_cls, $name)) {
      // __get is called if an existing property is inaccessible
      trigger_error("Cannot access property $static_cls::$name", E_USER_ERROR);
    }
    trigger_error("Undefined property $static_cls::$name", E_USER_NOTICE);
    return null;
  }

  // __get and __set are used to maintain read-only $this->name
  final public function __set(string $name, $value): void {
    // $name is a read-only property
    if ($name === 'name') {
      throw new ReflectionException(
        'Cannot set read-only property '.__CLASS__.'::'.$name);
    }
    if (property_exists(get_class($this), $name)) {
      // __set is called if the property is inaccessible
      trigger_error(
        'Cannot access property '.get_class($this).'::'.$name, E_USER_ERROR);
    }
    $this->{$name} = $value;
  }

  <<__Native>>
  private function getParentName(): string;

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.innamespace.php
   * )
   *
   * Checks if this class is defined in a namespace.
   *
   * @return     bool   Returns TRUE on success or FALSE on failure.
   */
  final public function inNamespace(): bool {
    return strrpos($this->getName(), '\\') !== false;
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionclass.functionabstract.php )
   *
   * Gets the namespace name.
   *
   * @return     string   The namespace name.
   */
  final public function getNamespaceName(): string {
    $name = $this->getName();
    $pos = strrpos($name, '\\');
    return ($pos === false) ? '' : substr($name, 0, $pos);
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionclass.getshortname.php )
   *
   * Get the short name of the function (without the namespace part).
   *
   * @return     string  The short name of the function.
   */
  final public function getShortName(): string {
    $name = $this->getName();
    $pos = strrpos($name, '\\');
    return ($pos === false) ? $name : substr($name, $pos + 1);
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.isinternal.php )
   *
   * Checks if the class is defined internally by an extension, or the core,
   * as opposed to user-defined.
   *
   * @return     bool   Returns TRUE on success or FALSE on failure.
   */
  <<__Native>>
  public function isInternal(): bool;

  public function isUserDefined(): bool {
    return !$this->isInternal();
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionclass.isinstantiable.php )
   *
   * Checks if the class is instantiable.
   *
   * @return     bool   Returns TRUE on success or FALSE on failure.
   */
  <<__Native>>
  public function isInstantiable(): bool;

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionclass.iscloneable.php )
   *
   * Returns whether this class is cloneable.
   *
   * @return     bool   Returns TRUE if the class is cloneable, FALSE otherwise.
   */
  public function isCloneable(): bool {
    return $this->isInstantiable() &&
      (!$this->hasMethod('__clone') || $this->getMethod('__clone')->isPublic());
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.getmethod.php )
   *
   * Gets a ReflectionMethod for a class method.
   *
   * @name       mixed   The method name to reflect.
   *
   * @return     mixed   A ReflectionMethod.
   */
  public function getMethod($name): ReflectionMethod {
    return new ReflectionMethod($this->getName(), $name);
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.hasmethod.php )
   *
   * Checks whether a specific method is defined in a class.
   *
   * @name       mixed   Name of the method being checked for.
   *
   * @return     bool    TRUE if it has the method, otherwise FALSE
   */
  <<__Native>>
  public function hasMethod(string $name): bool;

  /* Helper for getMethods: correctly ordered Set of the methods
   * declared on this class and its parents */
  <<__Native>>
  private function getMethodOrder(int $filter): object;

  private static $methOrderCache = array();
  private function getMethodOrderWithCaching(?int $filter): Set<string> {
    if (null === $filter) {
      $cached = hphp_array_idx(self::$methOrderCache, $this->getName(), null);
      if (null !== $cached) {
        return $cached;
      }
      return
        self::$methOrderCache[$this->getName()] = $this->getMethodOrder(0xFFFF);
    }
    return $this->getMethodOrder($filter);
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.getmethods.php )
   *
   * Gets an array of methods for the class.
   *
   * @filter     mixed   Filter the results to include only methods with
   *                     certain attributes. Defaults to no filtering.
   *
   *                     Any combination of ReflectionMethod::IS_STATIC,
   *                     ReflectionMethod::IS_PUBLIC,
   *                     ReflectionMethod::IS_PROTECTED,
   *                     ReflectionMethod::IS_PRIVATE,
   *                     ReflectionMethod::IS_ABSTRACT,
   *                     ReflectionMethod::IS_FINAL.
   *
   * @return     mixed   An array of ReflectionMethod objects reflecting each
   *                     method.
   */
  public function getMethods(?int $filter = null): array<ReflectionMethod> {
    $ret = array();
    $clsname = $this->getName();
    foreach ($this->getMethodOrderWithCaching($filter) as $name) {
      $ret[] = new ReflectionMethod($clsname, $name);
    }
    return $ret;
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.hasconstant.php
   * )
   *
   * Checks whether the class has a specific constant defined or not.
   *
   * @name       mixed   The name of the constant being checked for.
   *
   * @return     bool   TRUE if the constant is defined, otherwise FALSE.
   */
  <<__Native>>
  public function hasConstant(string $name): bool;

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.getconstant.php
   * )
   *
   * Gets the defined constant. Warning: This function is currently not
   * documented; only its argument list is available.
   *
   * @name       mixed   Name of the constant.
   *
   * @return     mixed   Value of the constant, or FALSE if it doesn't exist
   */
  <<__Native>>
  public function getConstant(string $name): mixed;

  private static $constCache = array();

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.getconstants.php
   * )
   *
   * Gets defined constants from a class. Warning: This function is
   * currently not documented; only its argument list is available.
   *
   * @return     array   An array of constants. Constant name in key,
   *                     constant value in value.
   */
  public function getConstants(): array<string, mixed> {
    $clsname = $this->getName();
    $cached = hphp_array_idx(self::$constCache, $clsname, null);
    if (null !== $cached) {
      return $cached;
    }
    return self::$constCache[$clsname] = $this->getOrderedConstants();
  }

  <<__Native>>
  private function getOrderedConstants(): array<string, mixed>;

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionclass.getinterfacenames.php )
   *
   * Get the interface names.
   *
   * @return     mixed   A numerical array with interface names as the
   *                     values.
   */
  <<__Native>>
  public function getInterfaceNames(): array<string>;

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionclass.getinterfaces.php )
   *
   * Gets the interfaces.
   *
   * @return     mixed   An associative array of interfaces, with keys as
   *                     interface names and the array values as
   *                     ReflectionClass objects.
   */
  public function getInterfaces(): array<string, ReflectionClass> {
    $ret = array();
    foreach ($this->getInterfaceNames() as $name) {
      $ret[$name] = new ReflectionClass($name);
    }
    return $ret;
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionclass.gettraitnames.php )
   *
   * Warning: This function is currently not documented; only its argument
   * list is available.
   *
   * @return     mixed   Returns an array with trait names in values. Returns
   *                     NULL in case of an error.
   */
  <<__Native>>
  public function getTraitNames(): array<string>;

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionclass.gettraitaliases.php )
   *
   * Warning: This function is currently not documented; only its argument
   * list is available.
   *
   * @return     mixed   Returns an array with new method names in keys and
   *                     original names (in the format "TraitName::original")
   *                     in values. Returns NULL in case of an error.
   */
  <<__Native>>
  public function getTraitAliases(): array<string, string>;

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.gettraits.php )
   *
   * Warning: This function is currently not documented; only its argument
   * list is available.
   *
   * @return     mixed   Returns an array with trait names in keys and
   *                     instances of trait's ReflectionClass in values.
   *                     Returns NULL in case of an error.
   */
  public function getTraits(): array<string, ReflectionClass> {
    $ret = array();
    foreach ($this->getTraitNames() as $name) {
      $ret[$name] = new ReflectionClass($name);
    }
    return $ret;
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.isinterface.php
   * )
   *
   * Checks whether the class is an interface.
   *
   * @return     mixed   Returns TRUE on success or FALSE on failure.
   */
  <<__Native>>
  public function isInterface(): bool;

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.isabstract.php )
   *
   * Checks if the class is abstract.
   *
   * @return     bool   Returns TRUE on success or FALSE on failure.
   */
  <<__Native>>
  public function isAbstract(): bool;

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.isfinal.php )
   *
   * Checks if a class is final.
   *
   * @return     bool   Returns TRUE on success or FALSE on failure.
   */
  <<__Native>>
  public function isFinal(): bool;

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.istrait.php )
   *
   * Warning: This function is currently not documented; only its argument
   * list is available.
   *
   * @return     bool   Returns TRUE on success or FALSE on failure.
   */
  <<__Native>>
  public function isTrait(): bool;

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.getmodifiers.php
   * )
   *
   * Returns a bitfield of the access modifiers for this class.
   *
   * @return     int   Returns bitmask of modifier constants.
   */
  <<__Native>>
  public function getModifiers(): int;

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.isinstance.php )
   *
   * Checks if an object is an instance of a class.
   *
   * @obj        mixed   The object being compared to.
   *
   * @return     bool   Returns TRUE on success or FALSE on failure.
   */
  public function isInstance($obj): bool {
    return is_a($obj, $this->getName());
  }

  <<__Native>>
  private function getConstructorName(): string;

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionclass.getconstructor.php )
   *
   * Gets the constructor of the reflected class.
   *
   * @return     mixed   A ReflectionMethod object reflecting the class'
   *                     constructor, or NULL if the class has no
   *                     constructor.
   */
  public function getConstructor(): ?ReflectionMethod {
    $constructor_name = $this->getConstructorName();
    return $constructor_name ? $this->getMethod($constructor_name): null;
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.newinstance.php
   * )
   *
   * Creates a new instance of the class. The given arguments are passed to
   * the class constructor.
   */
  public function newInstance(...$args) {
    if ($args && !$this->getConstructorName()) {
      // consistent with reference, but perhaps not particularly useful
      throw new ReflectionException(
        'Class '.$this->getName().' lacks a constructor, so you cannot pass'
        .' any constructor arguments'
      );
    }
    return hphp_create_object($this->getName(), $args);
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionclass.newinstanceargs.php )
   *
   * Creates a new instance of the class, the given arguments are passed to
   * the class constructor.
   *
   * @args       mixed   The parameters to be passed to the class constructor
   *                     as an array.
   *
   * @return     mixed   Returns a new instance of the class.
   */
  public function newInstanceArgs($args = array()) {
    if ($args && !$this->getConstructorName()) {
      // consistent with reference, but perhaps not particularly useful
      throw new ReflectionException(
        'Class '.$this->getName().' lacks a constructor, so you cannot pass'
        .' any constructor arguments'
      );
    }
    // XXX: is this array_values necessary?
    return hphp_create_object($this->getName(), array_values($args));
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionclass.newinstancewithoutconstructor.php
   * )
   *
   * Creates a new instance of the class without invoking the constructor.
   */
  public function newInstanceWithoutConstructor() {
    return hphp_create_object_without_constructor($this->getName());
  }

  // This calculations requires walking the preclasses in the hierarchy and
  // should not be getting performed repeatedly.
  <<__Native>>
  // returns array:
  //   'properties'               => array<string, prop_info_array>
  //   'private_properties'       => array<string, prop_info_array>
  //   'properties_index'         => array<string, int>
  //   'private_properties_index' => array<string, int>
  private function getClassPropertyInfo(): array;

  <<__Native>>
  private function getDynamicPropertyInfos(object $obj): array<string, mixed>;

  // Note: this cache could easily be shared between threads
  private static $propInfoCache = array();
  private function getOrderedPropertyInfos(): ConstMap<string, mixed> {
    $props_map = hphp_array_idx(self::$propInfoCache, $this->getName(), null);
    if (null === $props_map) {
      $prop_info = $this->getClassPropertyInfo();
      $properties = $prop_info['properties'] + $prop_info['private_properties'];
      $props_index =
        $prop_info['properties_index'] + $prop_info['private_properties_index'];
      $ordering = array_values($props_index);
      array_multisort($ordering, $properties);
      self::$propInfoCache[$this->getName()]
        = $props_map = new ImmMap($properties);
      // the $props_index does not need to be cached because $props are now
      // correctly ordered, we know that any dynamic properties with a name
      // collision will be appended
    }

    if (!$this->obj) { return $props_map; }

    // caching cannot be well applied to an object's dynamic properties,
    // since they can be added and removed at any time between calls to
    // property methods.
    $dynamic_props = $this->getDynamicPropertyInfos($this->obj);
    return (!$dynamic_props)
      ? $props_map
      : $props_map->toMap()->setAll($dynamic_props);
  }

  private function makeReflectionProperty(
    string $name,
    array $prop_info,
  ): ReflectionProperty {
    $ret = hphp_create_object_without_constructor(ReflectionProperty::class);
    $ret->name  = $name;
    $ret->info  = $prop_info;
    $ret->class = $this->getName();
    return $ret;
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.getproperty.php
   * )
   *
   * Gets a ReflectionProperty for a class's property.
   *
   * @name       mixed   The property name.
   *
   * @return     mixed   A ReflectionProperty.
   */
  public function getProperty($name) {
    $prop_info = $this->getOrderedPropertyInfos()->get($name);
    if (!$prop_info) {
      $class = $this->getName();
      throw new ReflectionException("Property $class::$name does not exist");
    }
    return $this->makeReflectionProperty($name, $prop_info);
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.hasproperty.php
   * )
   *
   * Checks whether the specified property is defined.
   *
   * @name       mixed  Name of the property being checked for.
   *
   * @return     bool   TRUE if it has the property, otherwise FALSE
   */
  public function hasProperty($name): bool {
    return $this->getOrderedPropertyInfos()->containsKey($name);
  }

  /**
   * ( excerpt* http://php.net/manual/en/reflectionclass.getproperties.php )
   *
   * Retrieves reflected properties.
   *
   * @filter     mixed   The optional filter, for filtering desired property
   *                     types. It's configured using the ReflectionProperty
   *                     constants, and defaults to all property types.
   *
   * @return     mixed   An array of ReflectionProperty objects.
   */
  public function getProperties($filter = 0xFFFF): array<ReflectionProperty> {
    $ret = array();
    foreach ($this->getOrderedPropertyInfos() as $name => $prop_info) {
      $p = $this->makeReflectionProperty($name, $prop_info);
      if (($filter & ReflectionProperty::IS_PUBLIC)    && $p->isPublic()    ||
          ($filter & ReflectionProperty::IS_PROTECTED) && $p->isProtected() ||
          ($filter & ReflectionProperty::IS_PRIVATE)   && $p->isPrivate()   ||
          ($filter & ReflectionProperty::IS_STATIC)    && $p->isStatic()) {
        $ret[] = $p;
      }
    }
    return $ret;
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionclass.getstaticproperties.php )
   *
   * Get the static properties. Warning: This function is currently not
   * documented; only its argument list is available.
   *
   * @return     mixed   The static properties, as an array.
   */
  public function getStaticProperties(): array<string, ReflectionProperty> {
    $ret = array();
    foreach ($this->getProperties(ReflectionProperty::IS_STATIC) as $prop) {
      $val = hphp_get_static_property($this->getName(), $prop->name, true);
      $ret[$prop->name] = $val;
    }
    return $ret;
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionclass.getstaticpropertyvalue.php )
   *
   * Gets the value of a static property on this class.
   *
   * @name       mixed   The name of the static property for which to return
   *                     a value.
   * @default    mixed
   *
   * @return     mixed   The value of the static property.
   */
  public function getStaticPropertyValue($name, $default = null) {
    if ($this->hasProperty($name) &&
        $this->getProperty($name)->isStatic()) {
      return hphp_get_static_property($this->getName(), $name, false);
    }
    return $default;
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionclass.setstaticpropertyvalue.php )
   *
   * Sets static property value. Warning: This function is currently not
   * documented; only its argument list is available.
   *
   * @name       mixed   Property name.
   * @value      mixed   New property value.
   */
  public function setStaticPropertyValue($name, $value): void {
    // XXX: should be __Native, not a builtin
    hphp_set_static_property($this->getName(), $name, $value, false);
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionclass.getdefaultproperties.php )
   *
   * Gets default properties from a class (including inherited properties).
   *
   * This method only works for static properties when used on internal
   * classes. The default value of a static class property can not be tracked
   * when using this method on user defined classes.
   *
   * @return     mixed   An array of default properties, with the key being
   *                     the name of the property and the value being the
   *                     default value of the property or NULL if the
   *                     property doesn't have a default value. The function
   *                     does not distinguish between static and non static
   *                     properties and does not take visibility modifiers
   *                     into account.
   */
  public function getDefaultProperties(): array<string, mixed> {
    $ret = array();
    foreach ($this->getProperties() as $prop) {
      if ($prop->isDefault()) {
        $ret[$prop->name] = $prop->info['defaultValue'];
      }
    }
    return $ret;
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.getextension.php
   * )
   *
   * Gets a ReflectionExtension object for the extension which defined the
   * class.
   *
   * @return     mixed   A ReflectionExtension object representing the
   *                     extension which defined the class, or NULL for
   *                     user-defined classes.
   */
  public function getExtension(): ?ReflectionExtension {
    // FIXME: HHVM doesn't support extension info
    return new ReflectionExtension($this->getExtensionName());
    // Truthful implementation would be:
    // return null;
    // A serious implementation would be:
    // return new ReflectionExtension($this->getExtensionName());
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionclass.getextensionname.php )
   *
   * Gets the name of the extension which defined the class.
   *
   * @return     mixed   The name of the extension which defined the class,
   *                     or FALSE for user-defined classes.
   */
  public function getExtensionName(): mixed {
    // FIXME: HHVM doesn't support extension info
    return '';
    // Truthful implementation would be:
    // return false;
    // A serious implementation would have this function __Native
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionclass.isiterateable.php )
   *
   * Checks whether the class is iterateable.
   *
   * @return     bool   Returns TRUE on success or FALSE on failure.
   */
  public function isIterateable(): bool {
    // XXX: is this correct? perhaps this should be Traversable?
    return $this->isSubclassOf(ArrayAccess::class);
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionclass.implementsinterface.php )
   *
   * Checks whether it implements an interface.
   *
   * @cls        mixed   The interface name.
   *
   * @return     bool    Returns TRUE on success or FALSE on failure.
   */
  public function implementsInterface($cls): bool {
    if ($cls instanceof ReflectionClass) { $cls = $cls->getName(); }

    // Normalize to avoid calling __autoload twice for undefined classes
    $normalized_cls = $cls[0] == '\\' ? substr($cls, 1) : $cls;
    if (!interface_exists($normalized_cls)) {
      throw new ReflectionException("Interface $normalized_cls does not exist");
    }
    return $this->isInterface()
      ? is_a($this->getName(), $normalized_cls, true)
      : $this->isSubclassOf($normalized_cls);
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionclass.getparentclass.php )
   *
   * Warning: This function is currently not documented; only its argument
   * list is available.
   *
   * @return     mixed   A ReflectionClass, or false.
   */
  public function getParentClass(): mixed {
    $parent = $this->getParentName();
    return $parent ? new ReflectionClass($parent) : false;
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.issubclassof.php
   * )
   *
   * Checks if the class is a subclass of a specified class or implements a
   * specified interface.
   *
   * @cls        mixed   The class name being checked against.
   *
   * @return     bool    Returns TRUE on success or FALSE on failure.
   */
  public function isSubclassOf($cls): bool {
    if ($cls instanceof ReflectionClass) {
      $cls = $cls->getName();
    }
    return is_subclass_of($this->getName(), $cls);
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.getfilename.php
   * )
   *
   * Gets the filename of the file in which the class has been defined.
   *
   * @return     mixed   Returns the filename of the file in which the class
   *                     has been defined. If the class is defined in the PHP
   *                     core or in a PHP extension, FALSE is returned.
   */
  <<__Native>>
  public function getFileName(): string;

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.getstartline.php
   * )
   *
   * Get the starting line number. Warning: This function is currently not
   * documented; only its argument list is available.
   *
   * @return     int   The starting line number, as an integer.
   */
  <<__Native>>
  public function getStartLine(): int;

  /**
   * ( excerpt from http://php.net/manual/en/reflectionclass.getendline.php )
   *
   * Gets end line number from a user-defined class definition.
   *
   * @return     int   The ending line number of the user defined class, or
   *                   FALSE if unknown.
   */
  <<__Native>>
  public function getEndLine(): int;

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionclass.getdoccomment.php )
   *
   * Gets doc comments from a class. Warning: This function is currently not
   * documented; only its argument list is available.
   *
   * @return     mixed   The doc comment if it exists, otherwise FALSE
   */
  <<__Native>>
  public function getDocComment(): mixed;

  public function getAttribute($name) {
    // Note: not particularly optimal ... could be a fast-terminating
    // __Native loop
    return hphp_array_idx($this->getAttributes(), $name, null);
  }

  <<__Native>>
  public function getAttributes(): array;

  public function getAttributeRecursive($name) {
    // Note: not particularly optimal ... could be a fast-terminating
    // __Native loop
    return hphp_array_idx($this->getAttributesRecursive(), $name, null);
  }

  <<__Native>>
  public function getAttributesRecursive(): array;
}

///////////////////////////////////////////////////////////////////////////////
// object

/**
 * ( excerpt from http://php.net/manual/en/class.reflectionobject.php )
 *
 * The ReflectionObject class reports information about an object.
 *
 */
class ReflectionObject extends ReflectionClass {

  /**
   * ( excerpt from http://www.php.net/manual/en/reflectionobject.construct.php
   * )
   *
   *  Constructs a ReflectionObject.
   */
  public function __construct($argument) {
    if (!is_object($argument)) {
      throw new ReflectionException(
        __CLASS__.' expects to be constructed with an object, got '
        .gettype($argument)
      );
    }
    parent::__construct($argument);
  }

  /**
   * ( excerpt from http://php.net/manual/en/reflectionobject.export.php )
   *
   * Exports a reflection. Warning: This function is currently not
   * documented; only its argument list is available.
   *
   * @obj        mixed   The reflection to export.
   * @ret        mixed   Setting to TRUE will return the export, as opposed
   *                     to emitting it. Setting to FALSE (the default) will
   *                     do the opposite.
   *
   * @return     mixed   If the return parameter is set to TRUE, then the
   *                     export is returned as a string, otherwise NULL is
   *                     returned.
   */
  public static function export($obj, $ret=false) {
    $obj = new ReflectionObject($obj);
    $str = (string)$obj;
    if ($ret) {
      return $str;
    }
    print $str;
  }
}
