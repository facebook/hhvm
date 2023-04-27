<?hh
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
  function get_defined_functions(): darray<string, varray<string>>;
  <<__PHPStdLib>>
  function function_exists(
    string $function_name,
    bool $autoload = true,
  )[]: bool;
  <<__PHPStdLib>>
  function is_callable(
    HH\FIXME\MISSING_PARAM_TYPE $v,
    bool $syntax = false,
  )[]: bool;
  <<__PHPStdLib>>
  function is_callable_with_name(
    HH\FIXME\MISSING_PARAM_TYPE $v,
    bool $syntax,
    inout HH\FIXME\MISSING_PARAM_TYPE $name,
  )[]: bool;
  <<__Deprecated('Use direct invocation instead.')>>
  function call_user_func_array(
    HH\FIXME\MISSING_PARAM_TYPE $function,
    Container<mixed> $params,
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__Deprecated('Use direct invocation instead.')>>
  function call_user_func(
    HH\FIXME\MISSING_PARAM_TYPE $function,
    HH\FIXME\MISSING_PARAM_TYPE ...$args
  ): \HH\FIXME\MISSING_RETURN_TYPE;
  <<__PHPStdLib>>
  function register_postsend_function<T>((function(): T) $function): void;
  <<__PHPStdLib>>
  function register_shutdown_function<T>((function(): T) $function): void;
}

namespace HH {
  function fun_get_function(mixed $fun)[]: string;
}
