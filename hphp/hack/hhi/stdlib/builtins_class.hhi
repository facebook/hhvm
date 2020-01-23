<?hh    /* -*- php -*- */
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
function get_declared_classes();
<<__PHPStdLib>>
function get_declared_interfaces();
<<__PHPStdLib>>
function get_declared_traits();
<<__PHPStdLib, __Rx>>
function enum_exists(string $class_name, bool $autoload = true): bool;
<<__PHPStdLib, __Rx>>
function class_exists(string $class_name, bool $autoload = true): bool;
<<__PHPStdLib, __Rx>>
function interface_exists(string $interface_name, bool $autoload = true): bool;
<<__PHPStdLib, __Rx>>
function trait_exists(string $trait_name, bool $autoload = true): bool;
<<__PHPStdLib>>
function get_class_methods($class_or_object): ?array<string>;
<<__PHPStdLib>>
function get_class_vars(string $class_name);
<<__Rx>>
function get_class(<<__MaybeMutable>> $object);
<<__Rx>>
function get_parent_class(<<__MaybeMutable>> $object = null);
<<__PHPStdLib, __Rx>>
function is_a(<<__MaybeMutable>> $class_or_object, string $class_name, bool $allow_string = false): bool;
<<__PHPStdLib, __Rx>>
function is_subclass_of($class_or_object, string $class_name, bool $allow_string = true): bool;
<<__PHPStdLib, __Rx>>
function method_exists($class_or_object, string $method_name): bool;
<<__PHPStdLib, __Rx>>
function property_exists($class_or_object, string $property): ?bool;
<<__Rx>>
function get_object_vars(<<__MaybeMutable>> $object): ?darray;
}

namespace HH {
<<__Rx>>
function class_meth_get_class(mixed $class_meth): string;
<<__Rx>>
function class_meth_get_method(mixed $class_meth): string;
<<__Rx>>
function meth_caller_get_class(mixed $meth_caller): string;
<<__Rx>>
function meth_caller_get_method(mixed $meth_caller): string;
}
