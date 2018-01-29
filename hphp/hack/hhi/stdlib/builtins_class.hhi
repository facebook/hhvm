<?hh // decl    /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
<<__PHPStdLib>>
function get_declared_classes();
<<__PHPStdLib>>
function get_declared_interfaces();
<<__PHPStdLib>>
function get_declared_traits();
<<__PHPStdLib>>
function enum_exists(string $class_name, bool $autoload = true): bool;
<<__PHPStdLib>>
function class_exists(string $class_name, bool $autoload = true): bool;
<<__PHPStdLib>>
function interface_exists(string $interface_name, bool $autoload = true): bool;
<<__PHPStdLib>>
function trait_exists(string $trait_name, bool $autoload = true): bool;
<<__PHPStdLib>>
function get_class_methods($class_or_object): ?array<string>;
<<__PHPStdLib>>
function get_class_vars(string $class_name);
function get_class($object = null);
function get_parent_class($object = null);
<<__PHPStdLib>>
function is_a($class_or_object, string $class_name, bool $allow_string = false): bool;
<<__PHPStdLib>>
function is_subclass_of($class_or_object, string $class_name, bool $allow_string = true): bool;
<<__PHPStdLib>>
function method_exists($class_or_object, string $method_name): bool;
<<__PHPStdLib>>
function property_exists($class_or_object, string $property): ?bool;
function get_object_vars($object): ?array;
function call_user_method_array(string $method_name, &$obj, array $paramarr);
function call_user_method(string $method_name, &$obj, ...);
