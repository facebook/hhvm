<?hh // decl /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */
function is_bool($var): bool;
function is_int($var): bool;
<<__Deprecated('Use is_int().')>>
function is_integer($var): bool;
<<__Deprecated('Use is_int().')>>
function is_long($var): bool;
<<__Deprecated('Use is_float().')>>
function is_double($var): bool;
function is_float($var): bool;
function is_numeric($var): bool;
<<__Deprecated('Use is_float().')>>
function is_real($var): bool;
function is_string($var): bool;
function is_scalar($var): bool;
function is_object($var): bool;
function is_resource($var): bool;
function is_null($var): bool;
function gettype($v);
function get_resource_type($handle);
function settype(&$var, $type);
function print_r($expression, $ret = false);
function var_export($expression, $ret = false);
function var_dump($expression, ...);
function debug_zval_dump($variable);
function serialize($value);
function unserialize($str, $class_whitelist = array());
function get_defined_vars();
function import_request_variables($types, $prefix = "");
function extract($var_array, $extract_type = EXTR_OVERWRITE, $prefix = "");
