<?hh /* -*- php -*- */
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

const int PHP_ROUND_HALF_UP;
const int PHP_ROUND_HALF_DOWN;
const int PHP_ROUND_HALF_EVEN;
const int PHP_ROUND_HALF_ODD;

const float M_PI;
const float M_E;
const float M_LOG2E;
const float M_LOG10E;
const float M_LN2;
const float M_LN10;
const float M_PI_2;
const float M_PI_4;
const float M_1_PI;
const float M_2_PI;
const float M_SQRTPI;
const float M_2_SQRTPI;
const float M_SQRT2;
const float M_SQRT3;
const float M_SQRT1_2;
const float M_LNPI;
const float M_EULER;

<<__PHPStdLib>>
function pi()[]: float;
<<__PHPStdLib>>
function min(
  HH\FIXME\MISSING_PARAM_TYPE $value,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function max(
  HH\FIXME\MISSING_PARAM_TYPE $value,
  HH\FIXME\MISSING_PARAM_TYPE ...$args
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function abs(
  HH\FIXME\MISSING_PARAM_TYPE $number,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function is_finite(float $val)[]: bool;
<<__PHPStdLib>>
function is_infinite(float $val)[]: bool;
<<__PHPStdLib>>
function is_nan(float $val)[]: bool;
<<__PHPStdLib>>
function ceil(
  HH\FIXME\MISSING_PARAM_TYPE $value,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function floor(HH\FIXME\MISSING_PARAM_TYPE $value)[]: float;
<<__PHPStdLib>>
function round(
  HH\FIXME\MISSING_PARAM_TYPE $val,
  int $precision = 0,
  int $mode = 1,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function deg2rad(float $number)[]: float;
<<__PHPStdLib>>
function rad2deg(float $number)[]: float;
<<__PHPStdLib>>
function decbin(
  HH\FIXME\MISSING_PARAM_TYPE $number,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function decoct(
  HH\FIXME\MISSING_PARAM_TYPE $number,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function bindec(
  HH\FIXME\MISSING_PARAM_TYPE $binary_string,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function hexdec(
  HH\FIXME\MISSING_PARAM_TYPE $hex_string,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function octdec(
  HH\FIXME\MISSING_PARAM_TYPE $octal_string,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function base_convert(
  HH\FIXME\MISSING_PARAM_TYPE $number,
  int $frombase,
  int $tobase,
)[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function pow(
  num $base,
  num $exp,
)[]: dynamic;
<<__PHPStdLib>>
function exp(float $arg)[]: float;
<<__PHPStdLib>>
function expm1(float $arg)[]: float;
<<__PHPStdLib>>
function log10(float $arg)[]: float;
<<__PHPStdLib>>
function log1p(float $number)[]: float;
<<__PHPStdLib>>
function log(float $arg, float $base = 0.0)[]: float;
<<__PHPStdLib>>
function cos(float $arg)[]: float;
<<__PHPStdLib>>
function cosh(float $arg)[]: float;
<<__PHPStdLib>>
function sin(float $arg)[]: float;
<<__PHPStdLib>>
function sinh(float $arg)[]: float;
<<__PHPStdLib>>
function tan(float $arg)[]: float;
<<__PHPStdLib>>
function tanh(float $arg)[]: float;
<<__PHPStdLib>>
function acos(float $arg)[]: float;
<<__PHPStdLib>>
function acosh(float $arg)[]: float;
<<__PHPStdLib>>
function asin(float $arg)[]: float;
<<__PHPStdLib>>
function asinh(float $arg)[]: float;
<<__PHPStdLib>>
function atan(float $arg)[]: float;
<<__PHPStdLib>>
function atanh(float $arg)[]: float;
<<__PHPStdLib>>
function atan2(float $y, float $x)[]: float;
<<__PHPStdLib>>
function hypot(float $x, float $y)[]: float;
<<__PHPStdLib>>
function fmod(float $x, float $y)[]: float;
<<__PHPStdLib>>
function sqrt(float $arg)[]: float;
<<__PHPStdLib>>
function getrandmax()[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function srand(
  HH\FIXME\MISSING_PARAM_TYPE $seed = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function rand(
  int $min = 0,
  HH\FIXME\MISSING_PARAM_TYPE $max = -1, /* getrandmax */
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mt_getrandmax()[]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mt_srand(
  HH\FIXME\MISSING_PARAM_TYPE $seed = null,
): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function mt_rand(
  int $min = 0,
  HH\FIXME\MISSING_PARAM_TYPE $max = -1, /* mt_getrandmax */
)[leak_safe]: HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function lcg_value(): HH\FIXME\MISSING_RETURN_TYPE;
<<__PHPStdLib>>
function intdiv(int $numerator, int $denominator)[]: int;
