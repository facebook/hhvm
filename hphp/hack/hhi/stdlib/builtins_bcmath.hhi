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
function bcscale(int $scale);
<<__PHPStdLib>>
function bcadd(string $left, string $right, int $scale = -1);
<<__PHPStdLib>>
function bcsub(string $left, string $right, int $scale = -1);
<<__PHPStdLib>>
function bccomp(string $left, string $right, int $scale = -1);
<<__PHPStdLib>>
function bcmul(string $left, string $right, int $scale = -1);
<<__PHPStdLib>>
function bcdiv(string $left, string $right, int $scale = -1);
<<__PHPStdLib>>
function bcmod(string $left, string $right);
<<__PHPStdLib>>
function bcpow(string $left, string $right, int $scale = -1);
<<__PHPStdLib>>
function bcpowmod(string $left, string $right, string $modulus, int $scale = -1);
<<__PHPStdLib>>
function bcsqrt(string $operand, int $scale = -1);
