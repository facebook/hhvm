<?hh
/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 *
 */

class A implements XHPChild {}

class :x:dumb extends XHPTest {}

function foo(mixed $x, string $s, ConstVector<string> $cs, dynamic $d): void {
  // This is allowed
  if ($x !== null) $s = $cs;
  $y = <x:dumb>{0}{'hello'}{$s}{new A()}{$d}</x:dumb>;
}
