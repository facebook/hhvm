<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
function get_defined_functions() { }
function function_exists($function_name, $autoload = true) { }
function is_callable($v, $syntax = false, &$name = null) { }
function call_user_func_array($function, $params) { }
function call_user_func($function, ...) { }
function call_user_func_array_async($function, $params) { }
function call_user_func_async($function, ...) { }
function check_user_func_async($handles, $timeout = -1) { }
function end_user_func_async($handle, $default_strategy = GLOBAL_STATE_IGNORE, $additional_strategies = null) { }
function forward_static_call_array($function, $params) { }
function forward_static_call($function, ...) { }
function get_called_class() { }
function create_function($args, $code) { }
function func_get_arg($arg_num) { }
function func_num_args() { }
function register_postsend_function($function, ...) { }
function register_shutdown_function($function, ...) { }
function register_cleanup_function($function, ...) { }
function register_tick_function($function, ...) { }
function unregister_tick_function($function_name) { }
