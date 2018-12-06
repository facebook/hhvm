<?hh   /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

// The functions in this file are defined in HHVM and known to the
// typechecker. There's no typechecker annotation syntax capable of
// describing how they are used to infer type info; these .hhi declarations
// are strictly for documentation purposes.

/**
 * See http://docs.hhvm.com/hack/reference/function/HH.fun/
 */
function fun(string $func_name); // becomes:
// function fun('something')
//   : (function(<the params of something>): <the return type of something>)

/**
 * See http://docs.hhvm.com/hack/reference/function/HH.meth_caller/
 */
function meth_caller(string $cls_name, string $meth_name); // becomes:
// function meth_caller(C::class or 'C', 'method')
//   : (function(C): <the return type of C::method>)

/**
 * See http://docs.hhvm.com/hack/reference/function/HH.class_meth/
 */
function class_meth(string $cls_name, string $meth_name); // becomes:
// function class_meth(C::class or 'C', 'method')
//   : (function(<params of C::method>): <the return type of C::method>)

/**
 * See http://docs.hhvm.com/hack/reference/function/HH.inst_meth/
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
  \HH\FormatString<PlainSprintf> $f, ...$f_args
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
  (function(string, ...): void) $callback
): void {}
