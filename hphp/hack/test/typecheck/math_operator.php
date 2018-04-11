<?hh // strict
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

function foo(): void {

  // Integer Operations
  $a = 1;
  $b = 2;
  $c = $a + $b;
  $c = $a - $b;
  $c = $a * $b;
  $c = $a % $b;
  $c = - $a;
  $c += $a;
  $c -= $a;
  $c *= $a;
  $c %= $a;
  ++$c;
  $c++;
  --$c;
  $c--;

  $c = $a ** $b;

  // Bitwise Operators
  $c = $a ^ $b;
  $c = $a & $b;
  $c = $a | $b;
  $c = $a << $b;
  $c = $a >> $b;
  $c &= $b;
  $c |= $b;
  $c <<= $b;
  $c >>= $b;

  // Boolean Operators
  $e = false;
  $f = true;
  $str = "1";
  $d = ($a == $b);
  $d = ($a != $b);
  // $d = ($a <> $b);
  $d = ($a !== $b);
  $d = ($a === $b);
  $d = ($a < $b);
  $d = ($a > $b);
  $d = ($a <= $b);
  $d = ($a >= $b);
  $d = !$e;
  $d = $a && $b;
  $d = $a || $b;

  // Spaceship operator
  $i = $a <=> $b;

  // Float operations
  $float_1 = 4.0;
  $float_2 = 3.0;
  $float_3 = $float_1/$float_2;
  $float_3 /= $float_1;
  $float_4 = $float_1 ** $float_3;
}
