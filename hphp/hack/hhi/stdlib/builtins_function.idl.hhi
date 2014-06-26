<?hh // decl
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
function get_defined_functions(): array<string, array<string>>;
function function_exists(string $function_name, $autoload = true): bool;
function is_callable($v, $syntax = false, &$name = null): bool;
function call_user_func_array<T>($function, Container<T> $params);
function call_user_func($function, ...);
function call_user_func_array_async<T>($function, Container<T> $params);
function call_user_func_async($function, ...);
function check_user_func_async($handles, $timeout = -1);
function end_user_func_async($handle, $default_strategy = GLOBAL_STATE_IGNORE, $additional_strategies = null);
function forward_static_call_array<T>($function, Container<T> $params);
function forward_static_call($function, ...);
function get_called_class(): string; // false if called from outside class
function create_function($args, $code);
function func_get_arg(int $arg_num);
function func_num_args(): int;
function register_postsend_function($function, ...);
function register_shutdown_function($function, ...);
function register_tick_function($function, ...);
function unregister_tick_function($function_name);
