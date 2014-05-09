<?hh

/**
 * ( excerpt from
 * http://php.net/manual/en/class.reflectionfunctionabstract.php )
 *
 * A parent class to ReflectionFunction and ParentMethod. Read their
 * descriptions for details.
 */
<<__NativeData('ReflectionFuncHandle')>>
abstract class ReflectionFunctionAbstract {

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
   * ( excerpt from
   * http://php.net/manual/en/reflectionfunctionabstract.getnamespacename.php )
   *
   * Gets the namespace name where the class is defined. Warning: This function
   * is currently not documented; only its argument list is available.
   *
   * @return     mixed   The namespace name.
   */
  final public function getNamespaceName(): string {
    $name = $this->getName();
    $pos = strrpos($name, '\\');
    if ($pos === false) {
      return '';
    }
    return substr($name, 0, $pos);
  }

  /**
   * ( excerpt from
   * http://php.net/manual/en/reflectionfunctionabstract.getshortname.php )
   *
   * Get the short name of the function (without the namespace part).
   *
   * @return     mixed   The short name of the function.
   */
  final public function getShortName(): string {
    $name = $this->getName();
    $pos = strrpos($name, '\\');
    if ($pos === false) {
      return $name;
    }
    return substr($name, $pos + 1);
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
class ReflectionFunction
extends ReflectionFunctionAbstract
implements Reflector {

  public string $name;
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
    $this->name = $this->getName();
  }

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
    return ($this->closure) ? '{closure}' : parent::getName();
  }

  <<__Native>>
  private function __initClosure(object $closure): void;

  <<__Native>>
  private function __initName(string $name): bool;

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

  // This doc comment block generated by idl/sysdoc.php
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

  // This doc comment block generated by idl/sysdoc.php
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
class ReflectionMethod
  extends ReflectionFunctionAbstract
  implements Reflector {

  public string $name;
  public string $class;

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

    if (!$this->__init($cls, $name)) {
      throw new ReflectionException("Method $classname::$name does not exist");
    }
    $this->originalClass = $classname;
    $this->name = $this->getName(); // $name might not have right caps
    $this->class = $this->getDeclaringClassname();
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
          $this->class, $this->name,
        )
      );
    }
    if ($this->isStatic()) {
      // Docs says to pass null, but Zend completely ignores the argument
      $obj = null;
    }
    return hphp_invoke_method($obj, $this->originalClass, $this->name, $args);
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
    return hphp_invoke_method($obj, $this->originalClass, $this->name,
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
      if (!($object instanceof $this->class)) {
        throw new ReflectionException(
          'Given object is not an instance of the class this method was '.
          'declared in' // mention $this->class / $this->originalClass here ?
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
    // XXX: could be more efficient using getPrototypeClassname?
    $p = get_parent_class($this->class);
    if ($p === false) {
      return null;
    }
    $rm = new ReflectionMethod($p, $this->name);
    if ($rm->isPrivate()) {
      return null;
    }
    return $rm->getAttributeRecursive($name);
  }

  public function getAttributesRecursive(): array {
    $attrs = $this->getAttributes();
    // XXX: could be more efficient using getPrototypeClassname?
    $p = get_parent_class($this->class);
    if ($p !== false) {
      $rm = new ReflectionMethod($p, $this->name);
      if (!$rm->isPrivate()) {
        $attrs += $rm->getAttributesRecursive();
      }
    }
    return $attrs;
  }
}
