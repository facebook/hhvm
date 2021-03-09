<?hh
// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

enum class E : int {
  int A = 42;
  int B = 1664;
}

enum class F : int extends E {
  int C = 16;
}

function test_e(HH\MemberOf<E, int> $x) : int {
  switch ($x) {
  case E::A: return 0;
  case E::B: return 1;
  }
}

function test_f(HH\MemberOf<F, int> $x) : int {
  switch ($x) {
  case F::A: return 0;
  case F::B: return 1;
  case F::C: return 2;
  }
}
