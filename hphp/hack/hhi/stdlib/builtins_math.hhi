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

<<__PHPStdLib>>
function pi()[];
<<__PHPStdLib>>
function min($value, ...$args)[];
<<__PHPStdLib>>
function max($value, ...$args)[];
<<__PHPStdLib>>
function abs($number)[];
<<__PHPStdLib>>
function is_finite(float $val)[];
<<__PHPStdLib>>
function is_infinite(float $val)[];
<<__PHPStdLib>>
function is_nan(float $val)[]: bool;
<<__PHPStdLib>>
function ceil($value)[];
<<__PHPStdLib>>
function floor($value)[]: float;
<<__PHPStdLib>>
function round($val, int $precision = 0, int $mode = 1)[];
<<__PHPStdLib>>
function deg2rad(float $number)[];
<<__PHPStdLib>>
function rad2deg(float $number)[];
<<__PHPStdLib>>
function decbin($number)[];
<<__PHPStdLib>>
function decoct($number)[];
<<__PHPStdLib>>
function bindec($binary_string)[];
<<__PHPStdLib>>
function hexdec($hex_string)[];
<<__PHPStdLib>>
function octdec($octal_string)[];
<<__PHPStdLib>>
function base_convert($number, int $frombase, int $tobase)[];
<<__PHPStdLib>>
function pow($base, $exp)[];
<<__PHPStdLib>>
function exp(float $arg)[];
<<__PHPStdLib>>
function expm1(float $arg)[];
<<__PHPStdLib>>
function log10(float $arg)[];
<<__PHPStdLib>>
function log1p(float $number)[];
<<__PHPStdLib>>
function log(float $arg, float $base = 0.0)[];
<<__PHPStdLib>>
function cos(float $arg)[];
<<__PHPStdLib>>
function cosh(float $arg)[];
<<__PHPStdLib>>
function sin(float $arg)[];
<<__PHPStdLib>>
function sinh(float $arg)[];
<<__PHPStdLib>>
function tan(float $arg)[];
<<__PHPStdLib>>
function tanh(float $arg)[];
<<__PHPStdLib>>
function acos(float $arg)[];
<<__PHPStdLib>>
function acosh(float $arg)[];
<<__PHPStdLib>>
function asin(float $arg)[];
<<__PHPStdLib>>
function asinh(float $arg)[];
<<__PHPStdLib>>
function atan(float $arg)[];
<<__PHPStdLib>>
function atanh(float $arg)[];
<<__PHPStdLib>>
function atan2(float $y, float $x)[];
<<__PHPStdLib>>
function hypot(float $x, float $y)[];
<<__PHPStdLib>>
function fmod(float $x, float $y)[];
<<__PHPStdLib>>
function sqrt(float $arg)[];
<<__PHPStdLib>>
function getrandmax()[];
<<__PHPStdLib>>
function srand($seed = null);
<<__PHPStdLib>>
function rand(int $min = 0, $max = -1 /* getrandmax */ );
<<__PHPStdLib>>
function mt_getrandmax()[];
<<__PHPStdLib>>
function mt_srand($seed = null);
<<__PHPStdLib>>
function mt_rand(int $min = 0, $max = -1 /* mt_getrandmax */ );
<<__PHPStdLib>>
function lcg_value();
<<__PHPStdLib>>
function intdiv(int $numerator, int $denominator)[]: int;
