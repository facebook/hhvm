<?hh // decl   /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

// The functions in this file are defined in HHVM and known to the
// typechecker. There's no typechecker annotation syntax capable of
// describing how they are used to infer type info; these .hhi declarations
// are strictly for documentation purposes.

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
 *
 *   $v = Vector {
 *     Vector {1, 2, 3},
 *     Vector {1, 2}
 *   };
 *
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
 *
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
 *
 *   class C {
 *     public function isOdd(int $i): bool { return $i % 2 == 1; }
 *     public function filter(Vector<int> $data): Vector<int> {
 *       $callback = inst_meth($this, 'isOdd');
 *       return $data->filter($callback);
 *     }
 *   }
 */
function inst_meth($inst, string $meth_name); // becomes:
// function inst_meth<Tobj>(Tobj inst, 'method')
//   : (function(<params of Tobj::method>): <the return type of Tobj::method>)

/**
 * A way to have a variable type checked as a more specific type than it is
 * currently declared. A source transformation in the runtime modifies code
 * that looks like:
 *
 *   invariant(<condition>, 'sprintf format: %s %d', 'string', ...);
 *
 * ... is transformed to be:
 *
 *   if (!(<condition>)) { // an Exception is thrown
 *     invariant_violation('sprintf format: %s', 'string', ...);
 *   }
 *   // <condition> is known to be true in the code below
 *
 * See http://docs.hhvm.com/manual/en/hack.otherrulesandfeatures.invariant.php
 * for more information.
 */
function invariant(
  $condition, // e.g. is_int($x) or ($y instanceof SomeClass)
  \HH\FormatString<PlainSprintf> $f, ...$f_args
): void;
