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
  function get_declared_classes(): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function get_declared_interfaces(): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function get_declared_traits(): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function enum_exists(string $enum_name, bool $autoload = true)[]: bool;
  <<__PHPStdLib>>
  function class_exists(string $class_name, bool $autoload = true)[]: bool;
  <<__PHPStdLib>>
  function interface_exists(
    string $interface_name,
    bool $autoload = true,
  )[]: bool;
  <<__PHPStdLib>>
  function trait_exists(string $trait_name, bool $autoload = true)[]: bool;
  <<__PHPStdLib>>
  function get_class_methods(HH\FIXME\MISSING_PARAM_TYPE $class_or_object)[]: ?varray_or_darray<string>;
  <<__PHPStdLib>>
  function get_class_vars(string $class_name): \HH\FIXME\MISSING_RETURN_TYPE;
  function get_class(
    readonly HH\FIXME\MISSING_PARAM_TYPE $object,
  )[]: \HH\FIXME\MISSING_RETURN_TYPE;
  function get_parent_class(
    HH\FIXME\MISSING_PARAM_TYPE $object = null,
  )[]: \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function is_a(
    HH\FIXME\MISSING_PARAM_TYPE $class_or_object,
    string $class_name,
    bool $allow_string = false,
  )[]: bool;
  <<__PHPStdLib>>
  function is_subclass_of(
    HH\FIXME\MISSING_PARAM_TYPE $class_or_object,
    string $class_name,
    bool $allow_string = true,
  )[]: bool;
  <<__PHPStdLib>>
  function method_exists(
    HH\FIXME\MISSING_PARAM_TYPE $class_or_object,
    string $method_name,
  )[]: bool;
  <<__PHPStdLib>>
  function property_exists(
    HH\FIXME\MISSING_PARAM_TYPE $class_or_object,
    string $property,
  )[]: ?bool;
  function get_object_vars(HH\FIXME\MISSING_PARAM_TYPE $object)[]: darray<string, mixed>;
}

namespace HH {
  function class_meth_get_class(mixed $class_meth)[]: string;
  function class_meth_get_method(mixed $class_meth)[]: string;
  function meth_caller_get_class(mixed $meth_caller)[]: string;
  function meth_caller_get_method(mixed $meth_caller)[]: string;
  // Hack specific existence functions
  function module_exists(string $module_name, bool $autoload = true)[]: bool;
  function package_exists(string $name): bool;
}
