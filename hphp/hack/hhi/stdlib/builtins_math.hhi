<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const PHP_ROUND_HALF_UP = 1;
const PHP_ROUND_HALF_DOWN = 2;
const PHP_ROUND_HALF_EVEN = 3;
const PHP_ROUND_HALF_ODD = 4;

const M_PI = 3.14159265358979323846;
const M_E  = 2.7182818284590452354;
const M_LOG2E = 1.4426950408889634074;
const M_LOG10E = 0.43429448190325182765;
const M_LN2 = 0.69314718055994530942;
const M_LN10 = 2.30258509299404568402;
const M_PI_2 = 1.57079632679489661923;
const M_PI_4 = 0.78539816339744830962;
const M_1_PI = 0.31830988618379067154;
const M_2_PI = 0.63661977236758134308;
const M_SQRTPI = 1.77245385090551602729;
const M_2_SQRTPI = 1.12837916709551257390;
const M_SQRT2 = 1.41421356237309504880;
const M_SQRT3 = 1.73205080756887729352;
const M_SQRT1_2 = 0.70710678118654752440;
const M_LNPI = 1.14472988584940017414;
const M_EULER = 0.57721566490153286061;

<<__PHPStdLib, __Pure>>
function pi();
<<__PHPStdLib, __Pure>>
function min($value, ...$args);
<<__PHPStdLib, __Pure>>
function max($value, ...$args);
<<__PHPStdLib, __Pure>>
function abs($number);
<<__PHPStdLib, __Pure>>
function is_finite(float $val);
<<__PHPStdLib, __Pure>>
function is_infinite(float $val);
<<__PHPStdLib, __Pure>>
function is_nan(float $val): bool;
<<__PHPStdLib, __Pure>>
function ceil($value);
<<__PHPStdLib, __Pure>>
function floor($value): float;
<<__PHPStdLib, __Pure>>
function round($val, int $precision = 0, int $mode = 1);
<<__PHPStdLib, __Pure>>
function deg2rad(float $number);
<<__PHPStdLib, __Pure>>
function rad2deg(float $number);
<<__PHPStdLib, __Pure>>
function decbin($number);
<<__PHPStdLib, __Pure>>
function decoct($number);
<<__PHPStdLib, __Pure>>
function bindec($binary_string);
<<__PHPStdLib, __Pure>>
function hexdec($hex_string);
<<__PHPStdLib, __Pure>>
function octdec($octal_string);
<<__PHPStdLib, __Pure>>
function base_convert($number, int $frombase, int $tobase);
<<__PHPStdLib, __Pure>>
function pow($base, $exp);
<<__PHPStdLib, __Pure>>
function exp(float $arg);
<<__PHPStdLib, __Pure>>
function expm1(float $arg);
<<__PHPStdLib, __Pure>>
function log10(float $arg);
<<__PHPStdLib, __Pure>>
function log1p(float $number);
<<__PHPStdLib, __Pure>>
function log(float $arg, float $base = 0.0);
<<__PHPStdLib, __Pure>>
function cos(float $arg);
<<__PHPStdLib, __Pure>>
function cosh(float $arg);
<<__PHPStdLib, __Pure>>
function sin(float $arg);
<<__PHPStdLib, __Pure>>
function sinh(float $arg);
<<__PHPStdLib, __Pure>>
function tan(float $arg);
<<__PHPStdLib, __Pure>>
function tanh(float $arg);
<<__PHPStdLib, __Pure>>
function acos(float $arg);
<<__PHPStdLib, __Pure>>
function acosh(float $arg);
<<__PHPStdLib, __Pure>>
function asin(float $arg);
<<__PHPStdLib, __Pure>>
function asinh(float $arg);
<<__PHPStdLib, __Pure>>
function atan(float $arg);
<<__PHPStdLib, __Pure>>
function atanh(float $arg);
<<__PHPStdLib, __Pure>>
function atan2(float $y, float $x);
<<__PHPStdLib, __Pure>>
function hypot(float $x, float $y);
<<__PHPStdLib, __Pure>>
function fmod(float $x, float $y);
<<__PHPStdLib, __Pure>>
function sqrt(float $arg);
<<__PHPStdLib, __Pure>>
function getrandmax();
<<__PHPStdLib, __NonRx('Randomness')>>
function srand($seed = null);
<<__PHPStdLib, __NonRx('Randomness')>>
function rand(int $min = 0, $max = -1 /* getrandmax */ );
<<__PHPStdLib, __Pure>>
function mt_getrandmax();
<<__PHPStdLib, __NonRx('Randomness')>>
function mt_srand($seed = null);
<<__PHPStdLib, __NonRx('Randomness')>>
function mt_rand(int $min = 0, $max = -1 /* mt_getrandmax */ );
<<__PHPStdLib, __NonRx('Randomness')>>
function lcg_value();
<<__PHPStdLib, __Pure>>
function intdiv(int $numerator, int $denominator): int;
