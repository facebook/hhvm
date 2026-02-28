<?hh
// Copyright (c) 2022, Meta, Inc.
// All rights reserved.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

// The functions in this file only *look* like functions to static analysis,
// but are compiled to bespoke bytecodes by HackC.

namespace {
  /**
   * Determines whether or not a variable is "set." This can take a subscript
   * expression and checks whether a given index exists, e.g.:
   * ```
   * // Returns `false` if 42 doesn't exist at all, or maps to `null`
   * isset($d[42]);
   * ```
   */
  function isset(mixed $x)[]: bool;

  /**
   * Used to remove keys from arrays:
   *
   * ```
   * $d = dict[42 => true];
   * unset($d[42]);
   * var_dump($d); // prints an empty dict
   * ```
   *
   * Also can be used to unset object properties and variables, but this behavior
   * is disallowed by Hack.
   */
  function unset(mixed $x)[]: void;

  /**
   * Psuedo-function for echo.
   *
   * Note that echo is not a real function. It's a language construct.
   *
   * This just stores the signature of the pseudo-function.
   */
  <<__SupportDynamicType>>
  function echo(arraykey ...$args): void;
}

namespace HH {
  /**
   * Checks whether the input is a native class method pointer.
   * ```
   * HH\is_class_meth(Foo::bar<>); // true
   * HH\is_class_meth(bing<>); // false
   * ```
   */
  function is_class_meth(readonly mixed $arg)[]: bool;
}
