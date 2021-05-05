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
<<__PHPStdLib>>
function is_bool($var)[]: bool;
<<__PHPStdLib>>
function is_int($var)[]: bool;
<<__Deprecated('Use is_int().')>>
function is_integer($var)[]: bool;
<<__Deprecated('Use is_int().')>>
function is_long($var)[]: bool;
<<__Deprecated('Use is_float().')>>
function is_double($var)[]: bool;
<<__PHPStdLib>>
function is_float($var)[]: bool;
<<__PHPStdLib>>
function is_numeric($var)[]: bool;
<<__Deprecated('Use is_float().')>>
function is_real($var)[]: bool;
<<__PHPStdLib>>
function is_string($var)[]: bool;
function is_scalar($var)[]: bool;
function is_object($var)[]: bool;
<<__PHPStdLib>>
function is_resource($var)[]: bool;
function is_null($var)[]: bool;
<<__PHPStdLib>>
function gettype($v)[];
<<__PHPStdLib>>
function get_resource_type(resource $handle)[];
<<__PHPStdLib>>
function print_r($expression, bool $ret = false);
<<__PHPStdLib>>
function var_export($expression, bool $ret = false);
<<__PHPStdLib>>
function var_dump(<<__AcceptDisposable>> mixed $expression, mixed ...$rest);
<<__PHPStdLib>>
function debug_zval_dump(<<__AcceptDisposable>> $variable);
<<__PHPStdLib>>
function serialize($value)[defaults]: string;
<<__PHPStdLib>>
function serialize_pure($value)[]: string;
<<__PHPStdLib>>
function unserialize(string $str, darray $options = darray[])[defaults];
<<__PHPStdLib>>
function unserialize_pure(string $str, darray $options = darray[])[];
<<__PHPStdLib>>
function import_request_variables(string $types, string $prefix = "");
}

namespace HH\Lib\_Private\Native {
  function first<Tv>(

    Container<Tv> $container,
  )[]: ?Tv;

  function first_key<Tk as arraykey>(

    KeyedContainer<Tk, mixed> $container,
  )[]: ?Tk;

  function last<Tv>(

    Container<Tv> $container,
  )[]: ?Tv;

  function last_key<Tk as arraykey>(

    KeyedContainer<Tk, mixed> $container,
  )[]: ?Tk;
}
