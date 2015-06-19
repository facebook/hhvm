<?hh

/**
 * ( excerpt from
 * http://docs.hhvm.com/manual/en/class.reflectionfunctionabstract.php )
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
   * http://docs.hhvm.com/manual/en/reflectionfunctionabstract.getname.php )
   *
   * Get the name of the function. Warning: This function is currently not
   * documented; only its argument list is available.
   *
   * @return     string   The name of the function.
   */
  <<__Native>>
  public function getName(): string;

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionfunctionabstract.innamespace.php
   * )
   *
   * Checks whether a function is defined in a namespace.
   *
   * @return     bool   Returns TRUE on success or FALSE on failure.
   */
  public function inNamespace(): bool {
    return strrpos($this->getName(), '\\') !== false;
  }

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionfunctionabstract.getnamespacename.php )
   *
   * Get the namespace name where the function is defined.
   *
   * @return     string   The namespace name.
   */
  public function getNamespaceName(): string {
    $name = $this->getName();
    $pos = strrpos($name, '\\');
    return ($pos === false) ? '' : substr($name, 0, $pos);
  }

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionfunctionabstract.getshortname.php )
   *
   * Get the short name of the function (without the namespace part).
   *
   * @return     string  The short name of the function.
   */
  public function getShortName(): string {
    $name = $this->getName();
    $pos = strrpos($name, '\\');
    return ($pos === false) ? $name : substr($name, $pos + 1);
  }

  <<__Native, __HipHopSpecific>>
  public function isHack(): bool;

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionfunctionabstract.isinternal.php )
   *
   * Checks whether the function is internal, as opposed to user-defined.
   * Warning: This function is currently not documented; only its argument
   * list is available.
   *
   * @return     mixed   TRUE if it's internal, otherwise FALSE
   */
  <<__Native>>
  public function isInternal(): bool;

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionfunctionabstract.isclosure.php )
   *
   * Checks whether it's a closure. Warning: This function is currently not
   * documented; only its argument list is available.
   *
   * @return     bool   TRUE if it's a closure, otherwise FALSE
   */
  public function isClosure(): bool {
    return false;
  }

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionfunctionabstract.isgenerator.php )
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
   * http://docs.hhvm.com/manual/en/reflectionfunctionabstract.isuserdefined.php )
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
   * http://docs.hhvm.com/manual/en/reflectionfunctionabstract.getfilename.php )
   *
   * Gets the file name from a user-defined function. Warning: This function
   * is currently not documented; only its argument list is available.
   *
   * @return     mixed   The file name.
   */
  <<__Native>>
  public function getFileName(): mixed;

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionfunctionabstract.getstartline.php )
   *
   * Gets the starting line number of the function. Warning: This function
   * is currently not documented; only its argument list is available.
   *
   * @return     mixed   The starting line number.
   */
  <<__Native>>
  public function getStartLine(): mixed;

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionfunctionabstract.getendline.php )
   *
   * Get the ending line number. Warning: This function is currently not
   * documented; only its argument list is available.
   *
   * @return     mixed   The ending line number of the user defined function,
   *                     or FALSE if unknown.
   */
  <<__Native>>
  public function getEndLine(): mixed;

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionfunctionabstract.getdoccomment.php )
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
   * http://docs.hhvm.com/manual/en/reflectionfunctionabstract.getstaticvariables.php
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
   * http://docs.hhvm.com/manual/en/reflectionfunctionabstract.returnsreference.php
   * )
   *
   * Checks whether the function returns a reference. Warning: This function
   * is currently not documented; only its argument list is available.
   *
   * @return     mixed   TRUE if it returns a reference, otherwise FALSE
   */
  <<__Native>>
  public function returnsReference(): bool;

  <<__Native, __HipHopSpecific>>
  private function getReturnTypeHint(): string;

  <<__HipHopSpecific>>
  public function getReturnTypeText() {
    return $this->getReturnTypeHint() ?: false;
  }

  /**
   * ( excerpt from
   *   http://docs.hhvm.com/manual/en/reflectionclass.getattributes.php )
   *
   * Gets all attributes
   *
   * @return  array<arraykey, array<int, mixed>>
   */
  <<__Native>>
  final public function getAttributes(): array;

  /**
   * ( excerpt from
   *   http://docs.hhvm.com/manual/en/reflectionclass.getattribute.php )
   *
   * Returns all attributes with given key.
   *
   * @return  ?array<int, mixed>
   */
  final public function getAttribute(string $name) {
    return hphp_array_idx($this->getAttributes(), $name, null);
  }

  /**
   * ( excerpt from
   *   http://docs.hhvm.com/manual/en/reflectionclass.getattributes.php )
   *
   * Gets all attributes
   *
   * @return  array<arraykey, array<int, mixed>>
   */
  public function getAttributesRecursive(): array {
    return $this->getAttributes();
  }

  /**
   * ( excerpt from
   *   http://docs.hhvm.com/manual/en/reflectionclass.getattributerecursive.php
   * )
   *
   * Returns all attributes with given key from a class and its parents.
   *
   * @return array<arraykey, array<int, mixed>>
   */
  public function getAttributeRecursive($name) {
    return $this->getAttribute($name);
  }

  <<__Native>>
  public function getNumberOfParameters(): int;

  <<__Native>>
  private function getParamInfo(): array;

  private $params = null;

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionfunctionabstract.getparameters.php )
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
   * http://docs.hhvm.com/manual/en/reflectionfunctionabstract.getnumberofrequiredparameters.php
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

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionfunctionabstract.isdeprecated.php
   * )
   *
   * Returns whether the function is deprecated.
   */
  public function isDeprecated(): bool {
    return null !== $this->getAttribute('__Deprecated');
  }

  public function getExtension() {
    // FIXME: HHVM doesn't support this
    return null;
  }

  public function getExtensionName() {
    return null;
  }

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionfunctionabstract.getclosurescopeclass.php
   * )
   *
   * Returns the scope associated to the closure
   *
   * @return     mixed   Returns the class on success or NULL on failure.
   */
  public function getClosureScopeClass(): ?ReflectionClass {
    return null;
  }

  // Prevent cloning
  final public function __clone() {
    throw new BadMethodCallException(
      'Trying to clone an uncloneable object of class ' . get_class($this)
    );
  }

  // Implementation of __toString
  final protected function __toStringHelper(
    $type,
    array $preAttrs = [],
    array $funcAttrs = [],
  ): string {
    $ret = '';
    if ($doc = $this->getDocComment()) {
      $ret .= $doc . "\n";
    }
    $ret .= "$type [ <";
    if ($this->isInternal()) {
      $ret .= 'internal';
      if ($this->isDeprecated()) {
        $ret .= ', deprecated';
      }
      if ($extensionName = $this->getExtensionName()) {
        $ret .= ':' . $extensionName;
      }
    } else {
      $ret .= 'user';
    }
    if ($preAttrs) {
      $ret .= ', ' . implode(', ', $preAttrs);
    }
    $ret .= '> ';
    if ($funcAttrs) {
      $ret .= implode(' ', $funcAttrs) . ' ';
    }
    $ret .= ($type == 'Method') ? 'method ' : 'function ';
    if ($this->returnsReference()) {
      $ret .= '&';
    }
    $ret .= $this->getName() . " ] {\n";

    if ($this->getStartLine() > 0) {
      $ret .= "  @@ {$this->getFilename()} " .
              "{$this->getStartLine()} - {$this->getEndLine()}\n";
    }

    if ($this->isClosure()) {
      // TODO: Not enough info
    }

    $params = $this->getParameters();
    if (count($params) > 0) {
      $ret .= "\n  - Parameters [" . count($params) . "] {\n  ";
      foreach ($params as $param) {
        $ret .= '  '.str_replace("\n", "\n  ", $param."\n");
      }
      $ret .= "}\n";
    }

    $ret .= "}\n";
    return $ret;
  }
}


/**
 * ( excerpt from http://docs.hhvm.com/manual/en/class.reflectionfunction.php )
 *
 * The ReflectionFunction class reports information about a function.
 */
class ReflectionFunction extends ReflectionFunctionAbstract {

  public string $name; // should be readonly (PHP compatibility)
  private ?Closure $closure = null;

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionfunction.construct.php
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

  <<__Native>>
  private function __initClosure(object $closure): bool;

  <<__Native>>
  private function __initName(string $name): bool;

  /**
   * (excerpt from
   * http://docs.hhvm.com/manual/en/reflectionfunctionabstract.getname.php )
   *
   * Get the name of the function. Warning: This function is currently not
   * documented; only its argument list is available.
   *
   * @return     string   The name of the function.
   */
  public function getName(): string {
    if ($this->closure) {
      // Format: Closure$scope;hash
      $cls = get_class($this->closure);
      $ns_end = strrpos($cls, '\\');
      if ($ns_end !== false) {
        $ns_start = strpos($cls, '$') + 1;
        $ns = substr($cls, $ns_start, $ns_end - $ns_start);
        return $ns.'\\{closure}';
      }
      return '{closure}';
    }
    return parent::getName();
  }

  public function getClosure() {
    return $this->closure ?:
      function(...$args) { return $this->invokeArgs($args); };
  }

  public function isClosure(): bool {
    return (bool) $this->closure;
  }

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionfunction.tostring.php )
   *
   * @return     string  A representation of this ReflectionFunction.
   */
  public function __toString(): string {
    return $this->__toStringHelper($this->isClosure() ? 'Closure' : 'Function');
  }

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionfunction.export.php )
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
   * http://docs.hhvm.com/manual/en/reflectionfunctionabstract.getstaticvariables.php
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
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionfunction.invoke.php )
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
   * http://docs.hhvm.com/manual/en/reflectionfunction.invokeargs.php )
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

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionfunction.isdisabled.php )
   *
   * Checks if the function is disabled, via the disable_functions directive.
   *
   * @return     bool   TRUE if it's disable, otherwise FALSE
   */
  public function isDisabled(): bool {
    // FIXME: HHVM doesn't support the disable_functions directive.
    return false;
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

  <<__Native>>
  private function getClosureThisObject(object $closure): object;

  /**
   * Returns this pointer bound to closure.
   *
   * @return object|NULL Returns $this pointer. Returns NULL in case of
   * an error.
   */
  public function getClosureThis(): ?mixed {
    if ($this->closure) {
      return $this->getClosureThisObject($this->closure);
    }
    return null;
  }
}

/**
 * ( excerpt from http://docs.hhvm.com/manual/en/class.reflectionmethod.php )
 *
 * The ReflectionMethod class reports information about a method.
 */
class ReflectionMethod extends ReflectionFunctionAbstract {

  public string $name; // should be readonly (PHP compatibility)
  public string $class; // should be readonly (PHP compatibility)

  private /*string*/ $originalClass;
  private /*bool*/ $forcedAccessible = false;

  <<__Native>>
  private function __init(mixed $cls_or_obj, string $meth): bool;

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionmethod.construct.php )
   *
   * Constructs a new ReflectionMethod.
   *
   * Format 1:
   *
   * @cls        mixed   Classname or object (instance of the class) that
   *                     contains the method.
   * @name       string  Name of the method.
   *
   * Format 2:
   *
   * @class_and_method string  Class name and method, separated by ::
   */
  public function __construct(...) {
    $args = func_get_args();
    if (count($args) == 0 || count($args) > 2) {
      throw new Exception(
        'ReflectionMethod::__construct() takes either 1 or 2 arguments');
    }

    if (count($args) == 1) {
      $arr = explode('::', $args[0], 3);
      if (count($arr) !== 2) {
        $name = $args[0];
        throw new ReflectionException("$name is not a valid method name");
      }
      list($cls, $name) = $arr;
      $classname = $cls;
    } else {
      $cls = $args[0];
      $name = (string) $args[1];

      $classname = is_object($cls) ? get_class($cls) : $cls;
      $method = $args[1];
    }

    $this->originalClass = $classname;
    if (!$this->__init($cls, (string) $name)) {
      throw new ReflectionException(
        "Method $classname::$name() does not exist");
    }

    $this->name = $this->getName();
    $this->class = $this->getDeclaringClassname();
  }

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionmethod.tostring.php )
   *
   * @return     string  A string representation of this ReflectionMethod.
   */
  public function __toString(): string {
    $preAttrs = [];

    $decl_class = $this->getDeclaringClassname();
    if ($this->originalClass !== $decl_class) {
      $preAttrs[] = "inherits $decl_class";
    } else if ($proto_cls = $this->getPrototypeClassname()) {
      $preAttrs[] = interface_exists($proto_cls, false)
        ? 'implements '.$proto_cls
        : 'overrides '.$proto_cls;
    }

    if ($this->isConstructor()) {
      $preAttrs[] = 'ctor';
    }
    if ($this->isDestructor()) {
      $preAttrs[] = 'dtor';
    }

    $funcAttrs = [];
    if ($this->isAbstract()) {
      $funcAttrs[] = 'abstract';
    }
    if ($this->isFinal()) {
      $funcAttrs[] = 'final';
    }
    if ($this->isStatic()) {
      $funcAttrs[] = 'static';
    }

    if ($this->isPrivate()) {
      $funcAttrs[] = 'private';
    } elseif ($this->isProtected()) {
      $funcAttrs[] = 'protected';
    } else {
      $funcAttrs[] = 'public';
    }

    return $this->__toStringHelper('Method', $preAttrs, $funcAttrs);
  }

  public function __debuginfo() {
    return array('name' => $this->name, 'class' => $this->class);
  }

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionmethod.export.php )
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
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionmethod.invoke.php )
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
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionmethod.invokeargs.php
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
    if ($this->isStatic()) {
      $obj = null;
    } else {
      if (!$obj) {
        $name = $this->originalClass.'::'.$this->getName();
        throw new ReflectionException(
          "Trying to invoke non static method $name() without an object",
        );
      }

      if (!$obj instanceof $this->originalClass) {
        throw new ReflectionException(
          'Given object is not an instance of the class this '.
            'method was declared in',
        );
      }
    }

    return hphp_invoke_method($obj, $this->originalClass, $this->getName(),
                              array_values($args));
  }

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionmethod.isfinal.php )
   *
   * Checks if the method is final.
   *
   * @return     bool   TRUE if the method is final, otherwise FALSE
   */
  <<__Native>>
  public function isFinal(): bool;

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionmethod.isabstract.php
   * )
   *
   * Checks if the method is abstract.
   *
   * @return     bool   TRUE if the method is abstract, otherwise FALSE
   */
  <<__Native>>
  public function isAbstract(): bool;

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionmethod.ispublic.php )
   *
   * Checks if the method is public.
   *
   * @return     bool   TRUE if the method is public, otherwise FALSE
   */
  <<__Native>>
  public function isPublic(): bool;

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionmethod.isprotected.php
   * )
   *
   * Checks if the method is protected.
   *
   * @return     bool   TRUE if the method is protected, otherwise FALSE
   */
  <<__Native>>
  public function isProtected(): bool;

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionmethod.isprivate.php )
   *
   * Checks if the method is private. Warning: This function is currently
   * not documented; only its argument list is available.
   *
   * @return     bool   TRUE if the method is private, otherwise FALSE
   */
  <<__Native>>
  public function isPrivate(): bool;

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionmethod.isstatic.php )
   *
   * Checks if the method is static.
   *
   * @return     bool   TRUE if the method is static, otherwise FALSE
   */
  <<__Native>>
  public function isStatic(): bool;

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionmethod.isconstructor.php )
   *
   * Checks if the method is a constructor.
   *
   * @return     bool   TRUE if the method is a constructor, otherwise FALSE
   */
  <<__Native>>
  public function isConstructor(): bool;

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionmethod.isdestructor.php )
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
   * http://docs.hhvm.com/manual/en/reflectionmethod.getmodifiers.php )
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
   * http://docs.hhvm.com/manual/en/reflectionmethod.getprototype.php )
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
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionmethod.getclosure.php
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
          . ' to be object, ' . gettype($object) . ' given', E_WARNING);
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

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionmethod.setaccessible.php )
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
   * http://docs.hhvm.com/manual/en/reflectionmethod.getdeclaringclass.php )
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

  public function getAttributeRecursive($name) {
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
 * ( excerpt from http://docs.hhvm.com/manual/en/class.reflectionclass.php )
 *
 * The ReflectionClass class reports information about a class.
 */
<<__NativeData('ReflectionClassHandle')>>
class ReflectionClass implements Reflector {
  const int IS_IMPLICIT_ABSTRACT = 16;
  const int IS_EXPLICIT_ABSTRACT = 32;
  const int IS_FINAL = 64;

  public string $name; // should be readonly (PHP compatibility)
  private $obj = null;

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.construct.php )
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

    $this->name = $this->getName();
  }

  <<__Native>>
  private function __init(string $name): string;

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.tostring.php )
   *
   * @return     string  A string representation of this ReflectionClass.
   */
  public function __toString(): string {
    $ret = '';
    if ($docComment = $this->getDocComment()) {
      $ret .= $docComment . "\n";
    }
    if ($this instanceof ReflectionObject) {
      $ret .= 'Object of class [ ';
    } elseif ($this->isInterface()) {
      $ret .= 'Interface [ ';
    } elseif ($this->isTrait()) {
      $ret .= 'Trait [ ';
    } elseif ($this->isEnum()) {
      $ret .= 'Enum [ ';
    } else {
      $ret .= 'Class [ ';
    }
    if ($this->isInternal()) {
      $ret .= '<internal:';
      if ($extensionName = $this->getExtensionName()) {
        $ret .= ':' . $extensionName;
      }
      $ret .= '> ';
    } else {
      $ret .= '<user> ';
    }
    if ($this->isIterateable()) {
      $ret .= '<iterable> ';
    }
    if ($this->isInterface()) {
      $ret .= 'interface ';
    } elseif ($this->isTrait()) {
      $ret .= 'trait ';
    } elseif ($this->isEnum()) {
      $ret .= 'enum ';
    } else {
      if ($this->isAbstract()) {
        $ret .= 'abstract ';
      }
      if ($this->isFinal()) {
        $ret .= 'final ';
      }
      $ret .= 'class ';
    }
    $ret .= $this->getName();
    if ($parent = $this->getParentName()) {
      $ret .= " extends $parent";
    }
    if ($ifaces = $this->getInterfaceNames()) {
      if ($this->isInterface()) {
        $ret .= ' extends ';
      } else {
        $ret .= ' implements ';
      }
      $ret .= implode(', ', $ifaces);
    }
    $ret .= " ] {\n";
    if ($this->getStartLine() > 0) {
      $ret .= "  @@ {$this->getFilename()} " .
              "{$this->getStartLine()}-{$this->getEndLine()}\n";
    }

    $consts = $this->getConstants();
    $abs_consts = $this->getAbstractConstantNames();
    $ret .= "\n  - Constants [" . (count($consts) + count($abs_consts)) . "] {\n";
    foreach ($consts as $k => $v) {
      $ret .= '    Constant [ ' . gettype($v) . " $k {" . (string)$v . "} ]\n";
    }
    foreach ($abs_consts as $k) {
      $ret .= '    Abstract Constant [ '. $k ."]\n";
    }
    $ret .= "  }\n";

    /* Static Properties */
    $props = $this->getProperties();
    $numStaticProps = 0;
    $numDynamicProps = 0;
    foreach ($props as $prop) {
      if ($prop->isStatic()) {
        ++$numStaticProps;
      } elseif (!$prop->isDefault()) {
        ++$numDynamicProps;
      }
    }
    $ret .= "\n  - Static properties [{$numStaticProps}] {\n  ";
    foreach ($props as $prop) {
      if (!$prop->isStatic()) { continue;}
      $ret .= '  ' . str_replace("\n", "\n  ", (string)$prop);
    }
    $ret .= "}\n";

    /* Static Methods  */
    $funcs = $this->getMethods();
    $numStaticFuncs = 0;
    foreach ($funcs as $func) {
      if ($func->isStatic()) ++$numStaticFuncs;
    }
    $ret .= "\n  - Static methods [{$numStaticFuncs}] {\n";
    foreach ($funcs as $func) {
      if (!$func->isStatic()) continue;
      $ret .= '    ' . str_replace("\n", "\n    ",
                                   rtrim((string)$func, "\n")) . "\n";
    }
    $ret .= "  }\n";

    /* Declared Instance Properties */
    $numMemberProps = count($props) - ($numStaticProps + $numDynamicProps);
    $ret .= "\n  - Properties [{$numMemberProps}] {\n  ";
    foreach ($props as $prop) {
      if ($prop->isStatic()) continue;
      if (!$prop->isDefault()) continue;
      $ret .= '  ' . str_replace("\n", "\n  ", (string)$prop);
    }
    $ret .= "}\n";

    /* Dynamic Instance Properties */
    if ($numDynamicProps) {
      $ret .= "\n  - Dynamic Properties [{$numDynamicProps}] {\n  ";
      foreach ($props as $prop) {
        if ($prop->isStatic()) continue;
        if ($prop->isDefault()) continue;
        $ret .= '  ' . str_replace("\n", "\n  ", (string)$prop);
      }
      $ret .= "}\n";
    }

    /* Instance Methods */
    $numMemberFuncs = count($funcs) - $numStaticFuncs;
    $ret .= "\n  - Methods [{$numMemberFuncs}] {\n";
    foreach ($funcs as $func) {
      if ($func->isStatic()) continue;
      $ret .= '    ' . str_replace("\n", "\n    ",
                                   rtrim((string)$func, "\n")) . "\n";
    }
    $ret .= "  }\n";

    $ret .= "}\n";
    return preg_replace("/(^|\n)\s+(\n|$)/", "\n\n", $ret);
  }

  // Prevent cloning
  final public function __clone() {
    throw new BadMethodCallException(
      'Trying to clone an uncloneable object of class ReflectionClass'
    );
  }

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.export.php )
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

  <<__Native>>
  private function getParentName(): string;

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.innamespace.php
   * )
   *
   * Checks if this class is defined in a namespace.
   *
   * @return     bool   Returns TRUE on success or FALSE on failure.
   */
  public function inNamespace(): bool {
    return strrpos($this->getName(), '\\') !== false;
  }

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionclass.functionabstract.php )
   *
   * Gets the namespace name.
   *
   * @return     string   The namespace name.
   */
  public function getNamespaceName(): string {
    $name = $this->getName();
    $pos = strrpos($name, '\\');
    return ($pos === false) ? '' : substr($name, 0, $pos);
  }

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionclass.getshortname.php )
   *
   * Get the short name of the function (without the namespace part).
   *
   * @return     string  The short name of the function.
   */
  public function getShortName(): string {
    $name = $this->getName();
    $pos = strrpos($name, '\\');
    return ($pos === false) ? $name : substr($name, $pos + 1);
  }

  <<__Native, __HipHopSpecific>>
  public function isHack(): bool;

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.isinternal.php )
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
   * http://docs.hhvm.com/manual/en/reflectionclass.isinstantiable.php )
   *
   * Checks if the class is instantiable.
   *
   * @return     bool   Returns TRUE on success or FALSE on failure.
   */
  <<__Native>>
  public function isInstantiable(): bool;

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionclass.iscloneable.php )
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
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.getmethod.php )
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
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.hasmethod.php )
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
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.getmethods.php )
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
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.hasconstant.php
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
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.getconstant.php
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
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.getconstants.php
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

  private static $absConstCache = array();

  /**
   * ( excerpt from
   *   http://docs.hhvm.com/manual/en/reflectionclass.getabstractconstantnames.php
   * )
   *
   * Returns an array containing the names of abstract constants as both
   * keys and values.
   *
   * @return  array<string, string>
   */
  public function getAbstractConstantNames(): array<string, string> {
    $clsname = $this->getName();
    $cached = hphp_array_idx(self::$absConstCache, $clsname, null);
    if (null !== $cached) {
      return $cached;
    }
    return self::$absConstCache[$clsname] = $this->getOrderedAbstractConstants();
  }

  private static $typeConstCache = array();

  private function getTypeConstantNamesWithCaching(): array<string, string> {
    $clsname = $this->getName();
    $cached = hphp_array_idx(self::$typeConstCache, $clsname, null);
    if (null !== $cached) {
      return $cached;
    }
    return self::$typeConstCache[$clsname] = $this->getOrderedTypeConstants();
  }

  public function getTypeConstant(string $name): ReflectionTypeConstant {
    return new ReflectionTypeConstant($this->getName(), $name);
  }

  public function hasTypeConstant($name): bool {
    return array_key_exists($name, $this->getTypeConstantNamesWithCaching());
  }

  public function getTypeConstants(): array<ReflectionTypeConstant> {
    $ret = array();
    $class = $this->getName();
    foreach ($this->getTypeConstantNamesWithCaching() as $name) {
      $ret[] = new ReflectionTypeConstant($class, $name);
    }
    return $ret;
  }

  <<__Native>>
  private function getOrderedConstants(): array<string, mixed>;

  <<__Native>>
  private function getOrderedAbstractConstants(): array<string, string>;

  <<__Native>>
  public function getOrderedTypeConstants(): array<string, string>;

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionclass.getinterfacenames.php )
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
   * http://docs.hhvm.com/manual/en/reflectionclass.getinterfaces.php )
   *
   * Gets the interfaces.
   *
   * @return     mixed   An associative array of interfaces, with keys as
   *                     interface names and the array values as
   *                     ReflectionClass objects.
   */
  public function getInterfaces(): array<string, ReflectionClass> {
    return $this->getReflectionClassesFromNames($this->getInterfaceNames());
  }

  /**
   * Gets the list of implemented interfaces/inherited classes needed to
   * implement an interface / use a trait. Empty array for abstract and
   * concrete classes.
   */
  <<__Native>>
  public function getRequirementNames(): array<string>;

  /**
   * Gets ReflectionClass-es for the requirements of this class
   *
   * @return  An associative array of requirements, with keys as
   *          requirement names and the array values as ReflectionClass objects.
   */
  public function getRequirements(): array<string, ReflectionClass> {
    return $this->getReflectionClassesFromNames($this->getRequirementNames());
  }

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionclass.gettraitnames.php )
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
   * http://docs.hhvm.com/manual/en/reflectionclass.gettraitaliases.php )
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
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.gettraits.php )
   *
   * Warning: This function is currently not documented; only its argument
   * list is available.
   *
   * @return     mixed   Returns an array with trait names in keys and
   *                     instances of trait's ReflectionClass in values.
   *                     Returns NULL in case of an error.
   */
  public function getTraits(): array<string, ReflectionClass> {
    return $this->getReflectionClassesFromNames($this->getTraitNames());
  }

  /**
   * Helper for the get{Traits,Interfaces,Requirements} methods
   */
  private function getReflectionClassesFromNames(array<string> $names) {
    $ret = array();
    foreach ($names as $name) {
      $ret[$name] = new ReflectionClass($name);
    }
    return $ret;
  }

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.isinterface.php
   * )
   *
   * Checks whether the class is an interface.
   *
   * @return     mixed   Returns TRUE on success or FALSE on failure.
   */
  <<__Native>>
  public function isInterface(): bool;

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.isabstract.php )
   *
   * Checks if the class is abstract.
   *
   * @return     bool   Returns TRUE on success or FALSE on failure.
   */
  <<__Native>>
  public function isAbstract(): bool;

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.isfinal.php )
   *
   * Checks if a class is final.
   *
   * @return     bool   Returns TRUE on success or FALSE on failure.
   */
  <<__Native>>
  public function isFinal(): bool;

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.istrait.php )
   *
   * Returns whether this is a trait.
   *
   * @return     bool   Returns TRUE if this is a trait, FALSE otherwise.
   */
  <<__Native>>
  public function isTrait(): bool;

  /**
   * Returns whether this ReflectionClass represents an enum.
   *
   * @return     bool   Returns TRUE if this is an enum, FALSE otherwise.
   */
  <<__Native>>
  public function isEnum(): bool;

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.getmodifiers.php
   * )
   *
   * Returns a bitfield of the access modifiers for this class.
   *
   * @return     int   Returns bitmask of modifier constants.
   */
  <<__Native>>
  public function getModifiers(): int;

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.isinstance.php )
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
   * http://docs.hhvm.com/manual/en/reflectionclass.getconstructor.php )
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
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.newinstance.php
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
   * http://docs.hhvm.com/manual/en/reflectionclass.newinstanceargs.php )
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
   * http://docs.hhvm.com/manual/en/reflectionclass.newinstancewithoutconstructor.php
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

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.getproperty.php
   * )
   *
   * Gets a ReflectionProperty for a class's property.
   *
   * @name       mixed   The property name.
   *
   * @return     mixed   A ReflectionProperty.
   */
  public function getProperty($name) {
    $class = $this->name;
    if (!$this->hasProperty($name)) {
      throw new ReflectionException("Property $class::$name does not exist");
    }
    if ($this->obj) {
      return new ReflectionProperty($this->obj, $name);
    } else {
      return new ReflectionProperty($this->name, $name);
    }
  }

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.hasproperty.php
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
   * ( excerpt* http://docs.hhvm.com/manual/en/reflectionclass.getproperties.php )
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
      if ($this->obj) {
        $p = new ReflectionProperty($this->obj, $name);
      } else {
        $p = new ReflectionProperty($this->name, $name);
      }
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
   * http://docs.hhvm.com/manual/en/reflectionclass.getstaticproperties.php )
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
   * http://docs.hhvm.com/manual/en/reflectionclass.getstaticpropertyvalue.php )
   *
   * Gets the value of a static property on this class.
   *
   * @name       mixed   The name of the static property for which to return
   *                     a value.
   * @default    mixed
   *
   * @return     mixed   The value of the static property.
   */
  public function getStaticPropertyValue($name /*, $default */) {
    // We can't check if a parameter isn't passed,
    // we can only check its default value, but that fails
    // if I want to pass the default value.
    // Use func_get_args() for this.
    $args = func_get_args();
    if ($this->hasProperty($name) &&
        $this->getProperty($name)->isStatic()) {
      return hphp_get_static_property($this->getName(), $name, false);
    } else if (!array_key_exists(1, $args)) {
      throw new ReflectionException(
        sprintf("Class %s does not have a property named %s",
                $this->getName(), $name)
      );
    }
    return $args[1];
  }

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionclass.setstaticpropertyvalue.php )
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
   * http://docs.hhvm.com/manual/en/reflectionclass.getdefaultproperties.php )
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
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.getextension.php
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
   * http://docs.hhvm.com/manual/en/reflectionclass.getextensionname.php )
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
   * http://docs.hhvm.com/manual/en/reflectionclass.isiterateable.php )
   *
   * Checks whether the class is iterateable.
   *
   * @return     bool   Returns TRUE on success or FALSE on failure.
   */
  public function isIterateable(): bool {
    return $this->isSubclassOf(\Traversable::class);
  }

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionclass.implementsinterface.php )
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
   * http://docs.hhvm.com/manual/en/reflectionclass.getparentclass.php )
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
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.issubclassof.php
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
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.getfilename.php
   * )
   *
   * Gets the filename of the file in which the class has been defined.
   *
   * @return     mixed   Returns the filename of the file in which the class
   *                     has been defined. If the class is defined in the PHP
   *                     core or in a PHP extension, FALSE is returned.
   */
  <<__Native>>
  public function getFileName(): mixed;

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.getstartline.php
   * )
   *
   * Get the starting line number. Warning: This function is currently not
   * documented; only its argument list is available.
   *
   * @return     int   The starting line number, as an integer.
   */
  <<__Native>>
  public function getStartLine(): mixed;

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionclass.getendline.php )
   *
   * Gets end line number from a user-defined class definition.
   *
   * @return     int   The ending line number of the user defined class, or
   *                   FALSE if unknown.
   */
  <<__Native>>
  public function getEndLine(): mixed;

  /**
   * ( excerpt from
   * http://docs.hhvm.com/manual/en/reflectionclass.getdoccomment.php )
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
 * ( excerpt from http://docs.hhvm.com/manual/en/class.reflectionobject.php )
 *
 * The ReflectionObject class reports information about an object.
 *
 */
class ReflectionObject extends ReflectionClass {

  /**
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionobject.construct.php
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
   * ( excerpt from http://docs.hhvm.com/manual/en/reflectionobject.export.php )
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

///////////////////////////////////////////////////////////////////////////////
// type constant

/**
 * The ReflectionTypeConstant class reports information about an object.
 *
 */
<<__NativeData('ReflectionConstHandle')>>
class ReflectionTypeConstant implements Reflector {

  /**
   * Constructs a new ReflectionTypeConstant.
   *
   * @cls        mixed   Classname or object (instance of the class) that
   *                     contains the type constant.
   * @name       string  Name of the type constant.
   */
  public function __construct(mixed $cls, string $name) {
    if (!$this->__init($cls, (string) $name)) {
      $classname = is_object($cls) ? get_class($cls) : $cls;
      throw new ReflectionException(
        "Type Constant $classname::$name does not exist");
    }
  }

  /**
   * Get the name of the type constant.
   *
   * @return     string   The name of the type constant.
   */
  <<__Native>>
  public function getName(): string;

  /**
   * Checks if the type constant is abstract
   *
   * @return     bool   Returns TRUE on success or FALSE on failure.
   */
  <<__Native>>
  public function isAbstract(): bool;

  /**
   * Get the type assigned to this type constant as a string
   *
   * @return     NULL | string   The assigned type or null if is abstract
   */
  public function getAssignedTypeText(): ?string {
    return $this->getAssignedTypeHint() ?: null;
  }

  /**
   * Gets the declaring class for the reflected type constant.
   *
   * @return ReflectionClass   A ReflectionClass object of the class that the
   *                           reflected type constant is part of.
   */
  public function getDeclaringClass() {
    return new ReflectionClass($this->getDeclaringClassname());
  }

  public function __toString() {
    $abstract = $this->isAbstract() ? 'abstract ' : '';

    $val = $this->isAbstract() ? '' : " = {$this->getAssignedTypeText()}";

    return "TypeConstant [ {$abstract}const type {$this->getName()}{$val}]\n";
  }

  // Prevent cloning
  final public function __clone() {
    throw new BadMethodCallException(
      'Trying to clone an uncloneable object of class ReflectionTypeConstant'
    );
  }

  public static function export($cls, $name, $ret=false) {
    $obj = new self($cls, $name);
    $str = (string)$obj;
    if ($ret) {
      return $str;
    }
    print $str;
  }

  <<__Native>>
  private function __init(mixed $cls_or_obj, string $const): bool;

  <<__Native>>
  private function getAssignedTypeHint(): string;

  <<__Native>>
  private function getDeclaringClassname(): string;
}
