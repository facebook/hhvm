<?hh   /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace HH {

// The functions in this file are defined in HHVM and known to the
// typechecker. There's no typechecker annotation syntax capable of
// describing how they are used to infer type info; these .hhi declarations
// are strictly for documentation purposes.

/**
 * Create a function reference to a global function
 *
 * The global function `fun('func_name')` creates a reference to a global
 * function.
 *
 * The parameter `'func_name'` is a constant string with the full name of the
 * global function to reference.
 *
 * Hack provides a variety of methods that allow you to construct references to
 * methods for delegation.  The methods in this group are:
 *
 * - [`class_meth`](/hack/reference/function/HH.class_meth/) for static methods on a class
 * - [`fun`](/hack/reference/function/HH.fun/) for global functions
 * - [`inst_meth`](/hack/reference/function/HH.inst_meth/) for instance methods on a single object
 * - [`meth_caller`](/hack/reference/function/HH.meth_caller/) for an instance method where the instance will be determined later
 * - Or use anonymous code within a [lambda](/hack/lambdas/introduction) expression.
 *
 * # Example
 *
 * ```
 * <?hh // strict
 * $v = vec["Hello", " ", "World", "!"];
 *
 * // Each line below prints "Hello World!"
 * Vec\map($v, fun('printf'));
 * Vec\map($v, $x ==> { printf($x); });
 * ```
 *
 * @param $func_name A constant string with the name of the global method, including namespace if required.
 * @return $func A fully typed function reference to the global method.
 */
function fun(string $func_name); // becomes:
// function fun('something')
//   : (function(<the params of something>): <the return type of something>)

/**
* Create a function reference to an instance method that can be called on any
* instance of the same type
*
* The global function `meth_caller('cls_name', 'meth_name')` creates a reference
* to an instance method on the specified class.  This method can then be used
* to execute across a collection of objects of that class.
*
* To identify the class for the fucntion, use a class reference of the format
* `MyClassName::class`.
*
* Hack provides a variety of methods that allow you to construct references to
* methods for delegation.  The methods in this group are:
*
* - [`class_meth`](/hack/reference/function/HH.class_meth/) for static methods on a class
* - [`fun`](/hack/reference/function/HH.fun/) for global functions
* - [`inst_meth`](/hack/reference/function/HH.inst_meth/) for instance methods on a single object
* - [`meth_caller`](/hack/reference/function/HH.meth_caller/) for an instance method where the instance will be determined later
* - Or use anonymous code within a [lambda](/hack/lambdas/introduction) expression.
*
* # Example
*
* ```
* <?hh // strict
* $v = Vector { Vector { 1, 2, 3 }, Vector { 1, 2 }, Vector { 1 } };
*
* // Each result returns Vector { 3, 2, 1 };
* $result2 = $v->map(meth_caller(Vector::class, 'count'));
* $result3 = $v->map($x ==> $x->count());
* ```
* @param $cls_name A constant string with the name of the class, or
*                  a class reference using `FullClassName::class`.
* @param $meth_name A constant string with the name of the instance method.
* @return $func_ref A fully typed function reference to the instance method.
*/
function meth_caller(string $cls_name, string $meth_name); // becomes:
// function meth_caller(C::class or 'C', 'method')
//   : (function(C): <the return type of C::method>)

/**
* Create a function reference to a static method on a class
*
* The global function `class_meth('cls_name', 'meth_name')` creates a reference
* to a static method on the specified class.
*
* To identify the class you can specify either a constant string containing a
* fully qualified class name including namespace, or a class reference using
* `MyClassName::class`.
*
* Hack provides a variety of methods that allow you to construct references to
* methods for delegation.  The methods in this group are:
*
* - [`class_meth`](/hack/reference/function/HH.class_meth/) for static methods on a class
* - [`fun`](/hack/reference/function/HH.fun/) for global functions
* - [`inst_meth`](/hack/reference/function/HH.inst_meth/) for instance methods on a single object
* - [`meth_caller`](/hack/reference/function/HH.meth_caller/) for an instance method where the instance will be determined later
* - Or use anonymous code within a [lambda](/hack/lambdas/introduction) expression.
*
* # Example
*
* ```
* <?hh // strict
* class C {
*     public static function isOdd(int $i): bool { return $i % 2 == 1;}
* }
* $data = Vector { 1, 2, 3 };
*
* // Each result returns Vector { 1, 3 }
* $data->filter(class_meth('C', 'isOdd'));
* $data->filter(class_meth(C::class, 'isOdd'));
* $data->filter($n ==> { return C::isOdd($n); });
* ```
* @param $cls_name A constant string with the name of the class, or
*                  a class reference using `FullClassName::class`.
* @param $meth_name A constant string with the name of the static class method.
* @return $func_ref A fully typed function reference to the static class method.
 */
function class_meth(string $cls_name, string $meth_name); // becomes:
// function class_meth(C::class or 'C', 'method')
//   : (function(<params of C::method>): <the return type of C::method>)

/**
* Create a function reference to an instance method on an object
*
* The global function `inst_meth($inst, 'meth_name')` creates a reference
* to an instance method on the specified object instance.
*
* When using `inst_meth` all function calls will go to the single object
* instance specified.  To call the same function on a collection
* of objects of compatible types, use [`meth_caller`](/hack/reference/function/HH.meth_caller/).
*
* Hack provides a variety of methods that allow you to construct references to
* methods for delegation.  The methods in this group are:
*
* - [`class_meth`](/hack/reference/function/HH.class_meth/) for static methods on a class
* - [`fun`](/hack/reference/function/HH.fun/) for global functions
* - [`inst_meth`](/hack/reference/function/HH.inst_meth/) for instance methods on a single object
* - [`meth_caller`](/hack/reference/function/HH.meth_caller/) for an instance method where the instance will be determined later
* - Or use anonymous code within a [lambda](/hack/lambdas/introduction) expression.
*
* # Example
*
* ```
* <?hh
* class C {
*   public function isOdd(int $i): bool { return $i % 2 == 1; }
* }
*
* $C = new C();
* $data = Vector { 1, 2, 3 };
*
* // Each result returns Vector { 1, 3 }
* var_dump($data->filter(inst_meth($C, 'isOdd')));
* var_dump($data->filter($n ==> { return $C->isOdd($n); }));
* ```
* @param $inst The object whose method will be referenced.
* @param $meth_name A constant string with the name of the instance method.
* @return $func_ref A fully typed function reference to the instance method.
 */
function inst_meth($inst, string $meth_name); // becomes:
// function inst_meth<Tobj>(Tobj inst, 'method')
//   : (function(<params of Tobj::method>): <the return type of Tobj::method>)

/**
 * See http://docs.hhvm.com/hack/reference/function/HH.invariant/
 */
<<__Rx>>
function invariant(
  $condition, // e.g. is_int($x) or ($y instanceof SomeClass)
  FormatString<\PlainSprintf> $f, ...$f_args
): void; // becomes:
// if (!(<condition>)) { // an Exception is thrown
//   invariant_violation('sprintf format: %s', 'string', ...);
// }
// <condition> is known to be true in the code below

/**
 * See
 * http://docs.hhvm.com/hack/reference/function/HH.invariant_callback_register/
 */
function invariant_callback_register(
  (function(string, mixed ...): void) $callback
): void {}

} // namespace HH
