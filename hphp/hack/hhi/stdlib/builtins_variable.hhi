<?hh // decl /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

<<__Rx>>
function is_bool($var): bool;
<<__Rx>>
function is_int($var): bool;
<<__Deprecated('Use is_int().'), __Rx>>
function is_integer($var): bool;
<<__Deprecated('Use is_int().'), __Rx>>
function is_long($var): bool;
<<__Deprecated('Use is_float().'), __Rx>>
function is_double($var): bool;
<<__Rx>>
function is_float($var): bool;
<<__PHPStdLib, __Rx>>
function is_numeric($var): bool;
<<__Deprecated('Use is_float().'), __Rx>>
function is_real($var): bool;
<<__Rx>>
function is_string($var): bool;
<<__Rx>>
function is_scalar($var): bool;
<<__Rx>>
function is_object($var): bool;
<<__Rx>>
function is_resource($var): bool;
<<__Rx>>
function is_null($var): bool;
<<__PHPStdLib, __Rx>>
function gettype($v);
<<__PHPStdLib, __Rx>>
function get_resource_type($handle);
<<__PHPStdLib>>
function settype(&$var, $type);
<<__PHPStdLib>>
function print_r($expression, $ret = false);
<<__PHPStdLib>>
function var_export($expression, $ret = false);
<<__PHPStdLib>>
function var_dump(<<__AcceptDisposable>> $expression, <<__AcceptDisposable>> ...$rest);
<<__PHPStdLib>>
function debug_zval_dump(<<__AcceptDisposable>> $variable);
<<__PHPStdLib, __Rx>>
function serialize($value);
<<__PHPStdLib>>
function unserialize($str, $class_whitelist = array());
<<__Rx>>
function get_defined_vars();
<<__PHPStdLib>>
function import_request_variables($types, $prefix = "");
function extract(&$var_array, $extract_type = EXTR_OVERWRITE, $prefix = "");
