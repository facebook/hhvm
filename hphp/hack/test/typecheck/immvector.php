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

// Test that the typechecker can correctly handle ImmVector.

// Helpers

function h1(): ImmVector<string> {
  return ImmVector {'hello', 'world'};
}

function h2(int $k) : void {}

function h3(string $v) : void {}

// Test that ImmVector {} : ImmVector<int>

function emptyfv(): ImmVector<int> {
  return ImmVector {};
}

// Test array-like access.

function simple(ImmVector<int> $fv) : int {
  return $fv[0];
}

// Nested FVs.

function nested(): ImmVector<ImmVector<string>> {
  return ImmVector {h1(), ImmVector {'a', 'b', 'c'}, ImmVector {}};
}

// Foreach over a FV.

function sum(): int {
  $v = ImmVector {1, 2, 3, 4, 5};
  $s = 0;
  foreach ($v as $k) {
    $s += $k;
  }
  return $s;
}

// Foreach over a FV with both key and value.

function feach(ImmVector<string> $vec) : void {
  foreach ($vec as $k => $v) {
    h2($k);
    h3($v);
  }
}

// List syntax

function lsyntax(ImmVector<int> $fv) : void {
  list($a, $b) = $fv;
}

// Generic FVs are covariant

class A {
}

class B extends A {
}

function covariance(ImmVector<B> $fvb) : ImmVector<A> {
  return $fvb;
}



