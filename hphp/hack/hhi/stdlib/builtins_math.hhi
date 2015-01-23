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
const PHP_ROUND_HALF_UP = 0;
const PHP_ROUND_HALF_DOWN = 0;
const PHP_ROUND_HALF_EVEN = 0;
const PHP_ROUND_HALF_ODD = 0;

const M_PI = 3.1415926535898;
const M_E  = 2.718281828459;

function pi();
function min($value, ...);
function max($value, ...);
function abs($number);
function is_finite($val);
function is_infinite($val);
function is_nan(float $val): bool;
function ceil($value);
function floor($value): float;
function round($val, $precision = 0, $mode = 1);
function deg2rad($number);
function rad2deg($number);
function decbin($number);
function decoct($number);
function bindec($binary_string);
function hexdec($hex_string);
function octdec($octal_string);
function base_convert($number, $frombase, $tobase);
function pow($base, $exp);
function exp($arg);
function expm1($arg);
function log10($arg);
function log1p($number);
function log($arg, $base = 0);
function cos($arg);
function cosh($arg);
function sin($arg);
function sinh($arg);
function tan($arg);
function tanh($arg);
function acos($arg);
function acosh($arg);
function asin($arg);
function asinh($arg);
function atan($arg);
function atanh($arg);
function atan2($y, $x);
function hypot($x, $y);
function fmod($x, $y);
function sqrt($arg);
function getrandmax();
function srand($seed = null);
function rand($min = 0, $max = -1 /* getrandmax */ );
function mt_getrandmax();
function mt_srand($seed = null);
function mt_rand($min = 0, $max = -1 /* mt_getrandmax */ );
function lcg_value();
