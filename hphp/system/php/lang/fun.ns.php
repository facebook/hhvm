<?php

namespace __SystemLib {
  // systemlib can't have closures, so we get this...
  final class MethCallerHelper {
    private ?string $class;
    private ?string $method;
    public function __construct(string $class, string $method) {
      $this->class = $class;
      $this->method = $method;
    }
    public function __invoke($x, ...$args) {
      invariant(
        $x instanceof $this->class,
        'object must be an instance of ('.$this->class.'), instead it is ('.
        (\is_object($x) ? \get_class($x) : \gettype($x)).')'
      );
      return $x->{$this->method}(...$args);
    }
    public function getClassName(): string {
      return $this->class;
    }
    public function getMethodName(): string {
      return $this->method;
    }
  };
}

namespace HH {

/**
 * `fun` is a special function used to create a "pointer" to a function in a
 * typeable way.
 *
 * The typechecker disallows using strings as functions; you must instead use
 * `fun()` to make sure the typechecker looks up the function signature and
 * returns a proper function type.
 *
 * For example:
 *
 * ```
 * <?hh
 * $a = [1, 2, 3];
 *
 * $c = 'count';
 * $c($a); // Type error: cannot call a string.
 *
 * $c = fun('count');
 * $c($a); // Legal: by using fun(), $c is now a callable with the right type.
 * ```
 *
 * See also:
 *  - [`meth_caller`](/hack/reference/function/HH.meth_caller/)
 *  - [`class_meth`](/hack/reference/function/HH.class_meth/)
 *  - [`inst_meth`](/hack/reference/function/HH.inst_meth/)
 *
 * @param $s Function to look up. Must be a constant string.
 * @return A callback which will call `$s` when invoked, but has the proper Hack
 *         function signature.
 *
 * @guide /hack/callables/special-functions
 */
<<__IsFoldable>>
function fun(string $s) /* interpreted by the type checker as
                           (function(<hack figures this>): <and this>) */ {
  return $s;
}

/**
 * Like [`fun`](/hack/reference/function/HH.fun/), but with the purpose of
 * calling an instance method on any object of a certain class.
 *
 * With `fun` you'd pass in something like `'count'` and it'd call `count($x)`
 * on whatever you pass in. This, rather, will call `$x->count()` for whatever
 * _object_ `$x` you pass in, which must be of type `$class`.
 *
 * For example:
 *
 * ```
 * <?hh
 * $v = Vector {
 *   Vector {1, 2, 3},
 *   Vector {1, 2}
 * };
 * $v->map(meth_caller('Vector', 'count'))  // returns Vector {3, 2}
 * ```
 *
 * ...calls the `count` method on the inner vectors, and returns a vector
 * of the results of that.
 *
 * See also:
 *  - [`fun`](/hack/reference/function/HH.fun/)
 *  - [`class_meth`](/hack/reference/function/HH.class_meth/)
 *  - [`inst_meth`](/hack/reference/function/HH.inst_meth/)
 *
 * @param $class The class of the method to call. Must be a constant string.
 * @param $method The method of the class that will be called. Must be a
 *        constant string.
 * @return A callback which will call `$method` when invoked.
 *
 * @guide /hack/callables/special-functions
 */
function meth_caller(string $class, string $method) {
  return new \__SystemLib\MethCallerHelper($class, $method);
}

/**
 * Like [`fun`](/hack/reference/function/HH.fun/), but with the purpose of
 * calling static methods.
 *
 * Example:
 *
 * ```
 * <?hh
 * class C {
 *   public static function isOdd(int $i): bool { return $i % 2 == 1; }
 * }
 * $data = Vector { 1, 2, 3 };
 * $data->filter(class_meth('C', 'isOdd'));
 * ```
 *
 * See also:
 *  - [`fun`](/hack/reference/function/HH.fun/)
 *  - [`meth_caller`](/hack/reference/function/HH.meth_caller/)
 *  - [`inst_meth`](/hack/reference/function/HH.inst_meth/)
 *
 * @param $class The class of the static method to call. Must be a constant
*         string.
 * @param $method The static method of the class that will be called. Must be a
 *        constant string.
 * @return A callback which will call `$method` when invoked.
 *
 * @guide /hack/callables/special-functions
 */
<<__IsFoldable>>
function class_meth(string $class, string $method)
  /* : (function(<hack figures this>): <and this>) */ {
  return array($class, $method);
}

/**
 * Like [`fun`](/hack/reference/function/HH.fun/), but with the purpose of
 * calling an instance method on a specific object.
 *
 * Example:
 *
 * ```
 * <?hh
 *  class C {
 *   private function isOdd(int $i): bool { return $i % 2 == 1; }
 *   private function filter(Vector<int> $data): Vector<int> {
 *     $callback = inst_meth($this, 'isOdd');
 *     return $data->filter($callback);
 *   }
 * }
 * ```
 *
 * See also:
 *  - [`fun`](/hack/reference/function/HH.fun/)
 *  - [`meth_caller`](/hack/reference/function/HH.meth_caller/)
 *  - [`class_meth`](/hack/reference/function/HH.class_meth/)
 *
 * @param $instance Any class object.
 * @param $method Method to call on `$instance`. Must be a constant string.
 * @return A callback which will call `$method` when invoked.
 *
 * @guide /hack/callables/special-functions
 */
function inst_meth($instance, string $method)
  /* : (function(<hack figures this>): <and this>) */ {
  invariant(\is_object($instance), 'expecting an object');
  return array($instance, $method);
}

}
