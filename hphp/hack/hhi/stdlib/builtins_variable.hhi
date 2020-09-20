<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

namespace {
<<__PHPStdLib, __Pure>>
function is_bool($var): bool;
<<__PHPStdLib, __Pure>>
function is_int($var): bool;
<<__Deprecated('Use is_int().'), __Pure>>
function is_integer($var): bool;
<<__Deprecated('Use is_int().'), __Pure>>
function is_long($var): bool;
<<__Deprecated('Use is_float().'), __Pure>>
function is_double($var): bool;
<<__PHPStdLib, __Pure>>
function is_float($var): bool;
<<__PHPStdLib, __Pure>>
function is_numeric($var): bool;
<<__Deprecated('Use is_float().'), __Pure>>
function is_real($var): bool;
<<__PHPStdLib, __Pure>>
function is_string($var): bool;
<<__Pure>>
function is_scalar($var): bool;
<<__Pure>>
function is_object($var): bool;
<<__PHPStdLib, __Pure>>
function is_resource($var): bool;
<<__Pure>>
function is_null($var): bool;
<<__PHPStdLib, __Pure>>
function gettype($v);
<<__PHPStdLib, __Pure>>
function get_resource_type(resource $handle);
<<__PHPStdLib>>
function print_r($expression, bool $ret = false);
<<__PHPStdLib>>
function var_export($expression, bool $ret = false);
<<__PHPStdLib>>
function var_dump(<<__AcceptDisposable>> mixed $expression, mixed ...$rest);
<<__PHPStdLib>>
function debug_zval_dump(<<__AcceptDisposable>> $variable);
<<__PHPStdLib, __Pure>>
function serialize($value);
<<__PHPStdLib, __Pure>>
function unserialize(string $str, darray $options = darray[]);
<<__PHPStdLib>>
function import_request_variables(string $types, string $prefix = "");
}

namespace HH\Lib\_Private\Native {
  <<__Pure, __AtMostRxAsArgs>>
  function first<Tv>(
    <<__OnlyRxIfImpl(\HH\Rx\Traversable::class), __MaybeMutable>>
    Container<Tv> $container,
  ): ?Tv;

  <<__Pure, __AtMostRxAsArgs>>
  function first_key<Tk as arraykey>(
    <<__OnlyRxIfImpl(\HH\Rx\Traversable::class), __MaybeMutable>>
    KeyedContainer<Tk, mixed> $container,
  ): ?Tk;

  <<__Pure, __AtMostRxAsArgs>>
  function last<Tv>(
    <<__OnlyRxIfImpl(\HH\Rx\Traversable::class), __MaybeMutable>>
    Container<Tv> $container,
  ): ?Tv;

  <<__Pure, __AtMostRxAsArgs>>
  function last_key<Tk as arraykey>(
    <<__OnlyRxIfImpl(\HH\Rx\Traversable::class), __MaybeMutable>>
    KeyedContainer<Tk, mixed> $container,
  ): ?Tk;
}
