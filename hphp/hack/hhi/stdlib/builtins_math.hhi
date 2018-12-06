<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
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

<<__PHPStdLib, __Rx>>
function pi();
<<__PHPStdLib, __Rx>>
function min($value, ...);
<<__PHPStdLib, __Rx>>
function max($value, ...);
<<__PHPStdLib, __Rx>>
function abs($number);
<<__PHPStdLib, __Rx>>
function is_finite($val);
<<__PHPStdLib, __Rx>>
function is_infinite($val);
<<__PHPStdLib, __Rx>>
function is_nan(float $val): bool;
<<__PHPStdLib, __Rx>>
function ceil($value);
<<__PHPStdLib, __Rx>>
function floor($value): float;
<<__PHPStdLib, __Rx>>
function round($val, $precision = 0, $mode = 1);
<<__PHPStdLib, __Rx>>
function deg2rad($number);
<<__PHPStdLib, __Rx>>
function rad2deg($number);
<<__PHPStdLib, __Rx>>
function decbin($number);
<<__PHPStdLib, __Rx>>
function decoct($number);
<<__PHPStdLib, __Rx>>
function bindec($binary_string);
<<__PHPStdLib, __Rx>>
function hexdec($hex_string);
<<__PHPStdLib, __Rx>>
function octdec($octal_string);
<<__PHPStdLib, __Rx>>
function base_convert($number, $frombase, $tobase);
<<__PHPStdLib, __Rx>>
function pow($base, $exp);
<<__PHPStdLib, __Rx>>
function exp($arg);
<<__PHPStdLib, __Rx>>
function expm1($arg);
<<__PHPStdLib, __Rx>>
function log10($arg);
<<__PHPStdLib, __Rx>>
function log1p($number);
<<__PHPStdLib, __Rx>>
function log($arg, $base = 0);
<<__PHPStdLib, __Rx>>
function cos($arg);
<<__PHPStdLib, __Rx>>
function cosh($arg);
<<__PHPStdLib, __Rx>>
function sin($arg);
<<__PHPStdLib, __Rx>>
function sinh($arg);
<<__PHPStdLib, __Rx>>
function tan($arg);
<<__PHPStdLib, __Rx>>
function tanh($arg);
<<__PHPStdLib, __Rx>>
function acos($arg);
<<__PHPStdLib, __Rx>>
function acosh($arg);
<<__PHPStdLib, __Rx>>
function asin($arg);
<<__PHPStdLib, __Rx>>
function asinh($arg);
<<__PHPStdLib, __Rx>>
function atan($arg);
<<__PHPStdLib, __Rx>>
function atanh($arg);
<<__PHPStdLib, __Rx>>
function atan2($y, $x);
<<__PHPStdLib, __Rx>>
function hypot($x, $y);
<<__PHPStdLib, __Rx>>
function fmod($x, $y);
<<__PHPStdLib, __Rx>>
function sqrt($arg);
<<__PHPStdLib, __Rx>>
function getrandmax();
<<__PHPStdLib>>
function srand($seed = null);
<<__PHPStdLib>>
function rand($min = 0, $max = -1 /* getrandmax */ );
<<__PHPStdLib, __Rx>>
function mt_getrandmax();
<<__PHPStdLib>>
function mt_srand($seed = null);
<<__PHPStdLib>>
function mt_rand($min = 0, $max = -1 /* mt_getrandmax */ );
<<__PHPStdLib>>
function lcg_value();
<<__PHPStdLib, __Rx>>
function intdiv(int $numerator, int $denominator): int;
