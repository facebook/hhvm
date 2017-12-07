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
<<__PHPStdLib>>
function decbin($number);
<<__PHPStdLib>>
function decoct($number);
function bindec($binary_string);
function hexdec($hex_string);
<<__PHPStdLib>>
function octdec($octal_string);
function base_convert($number, $frombase, $tobase);
function pow($base, $exp);
function exp($arg);
<<__PHPStdLib>>
function expm1($arg);
function log10($arg);
<<__PHPStdLib>>
function log1p($number);
function log($arg, $base = 0);
function cos($arg);
<<__PHPStdLib>>
function cosh($arg);
function sin($arg);
<<__PHPStdLib>>
function sinh($arg);
function tan($arg);
<<__PHPStdLib>>
function tanh($arg);
function acos($arg);
<<__PHPStdLib>>
function acosh($arg);
function asin($arg);
<<__PHPStdLib>>
function asinh($arg);
function atan($arg);
<<__PHPStdLib>>
function atanh($arg);
function atan2($y, $x);
<<__PHPStdLib>>
function hypot($x, $y);
function fmod($x, $y);
function sqrt($arg);
function getrandmax();
<<__PHPStdLib>>
function srand($seed = null);
function rand($min = 0, $max = -1 /* getrandmax */ );
function mt_getrandmax();
function mt_srand($seed = null);
function mt_rand($min = 0, $max = -1 /* mt_getrandmax */ );
<<__PHPStdLib>>
function lcg_value();
function intdiv(int $numerator, int $denominator): int;
