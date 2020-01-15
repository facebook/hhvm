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
<<__PHPStdLib, __Rx>>
function function_exists(string $function_name, bool $autoload = true): bool;
<<__PHPStdLib, __Rx>>
function is_callable($v, bool $syntax = false): bool;
<<__PHPStdLib, __Rx>>
function is_callable_with_name($v, bool $syntax, inout $name): bool;
function call_user_func_array($function, Container<mixed> $params);
function call_user_func($function, ...$args);
<<__PHPStdLib>>
function register_postsend_function<T>((function(): T) $function): void;
<<__PHPStdLib>>
function register_shutdown_function<T>((function(): T) $function): void;
}

namespace HH {
<<__Rx>>
function fun_get_function(mixed $fun): string;
}
