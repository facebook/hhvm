<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

enum E: int {
  A = 1;
  B = 2;
}

enum F: int {
  C = 2;
  D = 3;
}

function TestIt(): void {
  $x = E::A;
  $y = F::C;
  if ($x === $y) {
    echo 'same';
  }
}
