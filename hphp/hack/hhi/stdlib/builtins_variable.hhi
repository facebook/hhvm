<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */
<<file: __EnableUnstableFeatures("readonly")>>

namespace {
  <<__PHPStdLib>>
  function is_bool(readonly mixed $var)[]: bool;
  <<__PHPStdLib>>
  function is_int(readonly mixed $var)[]: bool;
  <<__Deprecated('Use is_int().')>>
  function is_integer(HH\FIXME\MISSING_PARAM_TYPE $var)[]: bool;
  <<__Deprecated('Use is_int().')>>
  function is_long(HH\FIXME\MISSING_PARAM_TYPE $var)[]: bool;
  <<__Deprecated('Use is_float().')>>
  function is_double(HH\FIXME\MISSING_PARAM_TYPE $var)[]: bool;
  <<__PHPStdLib>>
  function is_float(readonly mixed $var)[]: bool;
  <<__PHPStdLib>>
  function is_numeric(readonly mixed $var)[]: bool;
  <<__Deprecated('Use is_float().')>>
  function is_real(HH\FIXME\MISSING_PARAM_TYPE $var)[]: bool;
  <<__PHPStdLib>>
  function is_string(readonly mixed $var)[]: bool;
  function is_scalar(readonly mixed $var)[]: bool;
  function is_object(readonly mixed $var)[]: bool;
  <<__PHPStdLib>>
  function is_resource(readonly mixed $var)[]: bool;
  function is_null(readonly mixed $var)[]: bool;
  <<__PHPStdLib>>
  function gettype(readonly mixed $v)[]: \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function get_resource_type(resource $handle)[]: \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function print_r(
    HH\FIXME\MISSING_PARAM_TYPE $expression,
    bool $ret = false,
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function print_r_pure(
    HH\FIXME\MISSING_PARAM_TYPE $expression,
  )[]: \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function var_export(
    HH\FIXME\MISSING_PARAM_TYPE $expression,
    bool $ret = false,
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function var_export_pure(
    HH\FIXME\MISSING_PARAM_TYPE $expression,
  )[]: \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function var_dump(
    <<__AcceptDisposable>> readonly mixed $expression,
    mixed ...$rest
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function debug_zval_dump(
    <<__AcceptDisposable>> HH\FIXME\MISSING_PARAM_TYPE $variable,
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  function debugger_dump(
    <<__AcceptDisposable>> mixed $variable,
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function serialize(HH\FIXME\MISSING_PARAM_TYPE $value)[defaults]: string;
  <<__PHPStdLib>>
  function serialize_pure(HH\FIXME\MISSING_PARAM_TYPE $value)[]: string;
  <<__PHPStdLib>>
  function unserialize(
    string $str,
    darray<arraykey, mixed> $options = dict[],
  )[defaults]: \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function unserialize_pure(
    string $str,
    darray<arraykey, mixed> $options = dict[],
  )[]: \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function import_request_variables(
    string $types,
    string $prefix = "",
  ): \HH\FIXME\MISSING_RETURN_TYPE;
}

namespace HH {
  <<__PHPStdLib>>
  function object_prop_array(/*object*/
    mixed $obj,
    bool $ignore_late_init = false,
  )[]: darray<arraykey, mixed>;
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
