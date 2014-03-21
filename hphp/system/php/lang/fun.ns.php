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
    public function __invoke($x) {
      invariant($x instanceof $this->class,
                'object must be an instance of ('.$this->class.
                ') instead it is ('.get_class($x).')');
      return $x->{$this->method}();
    }
  };
}

namespace HH {

/**
 * fun is a special function used to create a "pointer" to a function in a
 * typeable way.
 *
 * The argument of fun() must always be a constant string.
 */
function fun(string $s) /* interpreted by the type checker as
                           (function(<hack figures this>): <and this>) */ {
  return $s;
}

/**
 * Like fun, but with the purpose of calling methods. With fun you'd pass in
 * something like 'count' and it'd call count($x) on whatever you pass in.
 * This, rather, will call ->count($x) on whatever _object_ you pass in,
 * which must be of type $class.
 *
 * For example:
 * $v = Vector {
 *   Vector {1, 2, 3},
 *   Vector {1, 2}
 * };
 * $v->map(meth_caller('Vector', 'count'))  // returns Vector {3, 2}
 * ...calls the 'count' method on the inner vectors, and return a vector
 * of the results of that.
 *
 * Both arguments must be constant strings.
 */
function meth_caller(string $class, string $method) {
  return new \__SystemLib\MethCallerHelper($class, $method);
}

/**
 * Similar to fun, creates a "pointer" to a callable that calls a
 * static method of a class in a typeable way.
 *
 * Both arguments must be constant strings.
 *
 * Example:
 *   class C {
 *     public static function isOdd(int $i): bool { return $i % 2 == 1;}
 *   }
 *   $data = Vector { 1, 2, 3 };
 *   $data->filter(class_meth('C', 'isOdd'));
 */
function class_meth(string $class, string $method)
  /* : (function(<hack figures this>): <and this>) */ {
  return array($class, $method);
}

/**
 * Similar to fun, creates a "pointer" to the invocation of a method on an
 * instance in a typeable way.
 *
 * Both arguments of inst_meth must be be a constant strings.
 *
 * Example:
 *   class C {
 *     private function isOdd(int $i): bool { return $i % 2 == 1; }
 *     private function filter(Vector<int> $data): Vector<int> {
 *       $callback = inst_meth($this, 'isOdd');
 *       return $data->filter($callback);
 *     }
 *   }
 */
function inst_meth($instance, string $method)
  /* : (function(<hack figures this>): <and this>) */ {
  invariant(is_object($instance), 'expecting an object');
  return array($instance, $method);
}

}
