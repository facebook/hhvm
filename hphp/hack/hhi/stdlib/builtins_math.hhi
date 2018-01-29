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

const M_PI = 0.0;
const M_E  = 0.0;
const M_LOG2E = 0.0;
const M_LOG10E = 0.0;
const M_LN2 = 0.0;
const M_LN10 = 0.0;
const M_PI_2 = 0.0;
const M_PI_4 = 0.0;
const M_1_PI = 0.0;
const M_2_PI = 0.0;
const M_SQRTPI = 0.0;
const M_2_SQRTPI = 0.0;
const M_SQRT2 = 0.0;
const M_SQRT3 = 0.0;
const M_SQRT1_2 = 0.0;
const M_LNPI = 0.0;
const M_EULER = 0.0;

<<__PHPStdLib>>
function pi();
<<__PHPStdLib>>
function min($value, ...);
<<__PHPStdLib>>
function max($value, ...);
<<__PHPStdLib>>
function abs($number);
<<__PHPStdLib>>
function is_finite($val);
<<__PHPStdLib>>
function is_infinite($val);
<<__PHPStdLib>>
function is_nan(float $val): bool;
<<__PHPStdLib>>
function ceil($value);
<<__PHPStdLib>>
function floor($value): float;
<<__PHPStdLib>>
function round($val, $precision = 0, $mode = 1);
<<__PHPStdLib>>
function deg2rad($number);
<<__PHPStdLib>>
function rad2deg($number);
<<__PHPStdLib>>
function decbin($number);
<<__PHPStdLib>>
function decoct($number);
<<__PHPStdLib>>
function bindec($binary_string);
<<__PHPStdLib>>
function hexdec($hex_string);
<<__PHPStdLib>>
function octdec($octal_string);
<<__PHPStdLib>>
function base_convert($number, $frombase, $tobase);
<<__PHPStdLib>>
function pow($base, $exp);
<<__PHPStdLib>>
function exp($arg);
<<__PHPStdLib>>
function expm1($arg);
<<__PHPStdLib>>
function log10($arg);
<<__PHPStdLib>>
function log1p($number);
<<__PHPStdLib>>
function log($arg, $base = 0);
<<__PHPStdLib>>
function cos($arg);
<<__PHPStdLib>>
function cosh($arg);
<<__PHPStdLib>>
function sin($arg);
<<__PHPStdLib>>
function sinh($arg);
<<__PHPStdLib>>
function tan($arg);
<<__PHPStdLib>>
function tanh($arg);
<<__PHPStdLib>>
function acos($arg);
<<__PHPStdLib>>
function acosh($arg);
<<__PHPStdLib>>
function asin($arg);
<<__PHPStdLib>>
function asinh($arg);
<<__PHPStdLib>>
function atan($arg);
<<__PHPStdLib>>
function atanh($arg);
<<__PHPStdLib>>
function atan2($y, $x);
<<__PHPStdLib>>
function hypot($x, $y);
<<__PHPStdLib>>
function fmod($x, $y);
<<__PHPStdLib>>
function sqrt($arg);
<<__PHPStdLib>>
function getrandmax();
<<__PHPStdLib>>
function srand($seed = null);
<<__PHPStdLib>>
function rand($min = 0, $max = -1 /* getrandmax */ );
<<__PHPStdLib>>
function mt_getrandmax();
<<__PHPStdLib>>
function mt_srand($seed = null);
<<__PHPStdLib>>
function mt_rand($min = 0, $max = -1 /* mt_getrandmax */ );
<<__PHPStdLib>>
function lcg_value();
<<__PHPStdLib>>
function intdiv(int $numerator, int $denominator): int;
