<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

enum Enum: int {
  A = 0;
  B = 1;
  C = 2;
  D = 3;
}

function f(Enum $e): int {
  $x = null;
  switch ($e) {
    case Enum::A:
      $x = 1;
      break;
    case Enum::B:
    case Enum::C:
      $x = 2;
      break;
    case Enum::D:
      return 3;
  }
  return $x;
}
