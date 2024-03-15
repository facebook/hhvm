<?hh /* -*- php -*- */
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
  * Create a function reference to an instance method that can be called on any
  * instance of the same type
  *
  * The global function `meth_caller(C::class, 'meth_name')` creates a reference
  * to an instance method on the specified class.  This method can then be used
  * to execute across a collection of objects of that class.
  *
  * To identify the class for the function, use a class reference of the format
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
  * @param $cls_name A class reference using `FullClassName::class`.
  * @param $meth_name A constant string with the name of the instance method.
  * @return $func_ref A fully typed function reference to the instance method.
  */
  function meth_caller(
    classname<mixed> $cls_name,
    string $meth_name,
  ): \HH\FIXME\MISSING_RETURN_TYPE; // becomes:
  // function meth_caller(C::class, 'method')
  //   : (function(C): <the return type of C::method>)

  /**
   * Call `invariant_violation` if the condition is false.
   *
   * ```
   * invariant($x >= 0, "Value cannot be negative: %d", $x);
   * ```
   *
   * See http://docs.hhvm.com/hack/reference/function/HH.invariant/
   */
  function invariant(
    \HH\FIXME\MISSING_PARAM_TYPE $condition,
    FormatString<\PlainSprintf> $f,
    mixed ...$f_args
  )[]: void; // becomes:
  // if (!(<condition>)) { // an Exception is thrown
  //   invariant_violation('sprintf format: %s', 'string', ...);
  // }
  // <condition> is known to be true in the code below

  /**
   * See
   * http://docs.hhvm.com/hack/reference/function/HH.invariant_callback_register/
   */
  function invariant_callback_register(
    (function(string, mixed...): void) $callback,
  )[globals]: void {}

} // namespace HH
