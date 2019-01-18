<?hh     /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__PHPStdLib>>
function hphp_get_extension_info(string $name);
<<__PHPStdLib>>
function hphp_get_class_constant($cls, $name) { }
<<__PHPStdLib>>
function hphp_invoke(string $name, $params);
<<__PHPStdLib>>
function hphp_invoke_method($obj, string $cls, string $name, $params);
<<__PHPStdLib>>
function hphp_instanceof($obj, $name) { }
<<__PHPStdLib>>
function hphp_create_object_without_constructor(string $name);
<<__PHPStdLib>>
function hphp_get_property($obj, string $cls, string $prop);
<<__PHPStdLib>>
function hphp_set_property($obj, string $cls, string $prop, $value);
<<__PHPStdLib>>
function hphp_get_static_property(string $cls, string $prop, bool $force);
<<__PHPStdLib>>
function hphp_set_static_property(string $cls, string $prop, $value, bool $force);
<<__PHPStdLib>>
function hphp_scalar_typehints_enabled() { }
