<?hh // decl
// Copyright 2004-present Facebook. All Rights Reserved.

// The functions in this file are defined in HHVM and known to the
// typechecker. They do not appear in .hhi files because there's no
// typechecker annotation syntax that describes how they are used to infer
// type info.

/**
 * fun is a special function used to create an opaque "pointer" to a
 * function in a typeable way.
 *
 * The argument of fun() must always be a constant string. The typechecker
 * figures out the params and return value from that information.
 */
function fun(string $func_name); // becomes:
// function fun('something')
//   : (function(<the params of something>): <the return type of something>)

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
function meth_caller(string $cls_name, string $meth_name); // becomes:
// function meth_caller(C::class or 'C', 'method')
//   : (function(C): <the return type of C::method>)

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
function class_meth(string $cls_name, string $meth_name); // becomes:
// function class_meth(C::class or 'C', 'method')
//   : (function(<params of C::method>): <the return type of C::method>)

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
function inst_meth($inst, string $meth_name); // becomes:
// function inst_meth<Tobj>(Tobj inst, 'method')
//   : (function(<params of Tobj::method>): <the return type of Tobj::method>)
