<?hh
/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

enum class E : int {
  int A = 42;
  int B = 1664;
}

enum class F : int extends E {
  int C = 16;
}

function test_e(HH\EnumClass\Label<E, int> $x) : int {
  switch ($x) {
  case E#A: return 0;
  case E#B: return 1;
  }
}

function test_f(HH\EnumClass\Label<F, int> $x) : int {
  switch ($x) {
  case F#A: return 0;
  case F#B: return 1;
  case F#C: return 2;
  }
}
