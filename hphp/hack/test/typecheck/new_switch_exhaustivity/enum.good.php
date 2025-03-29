<?hh

enum E: int {
  A = 0;
  B = 1;
}

enum F: int {
  A = 0;
  B = 1;
}

function test_exhaustive(E $e): void {
  switch ($e) {
    case E::A:
      return;
    case E::B:
      return;
  }
}

function test_with_default(E $e): void {
  switch ($e) {
    case E::A:
      return;
    default:
      return;
  }
}

function test_wrong_enum_with_default(E $e): void {
  switch ($e) {
    case F::A:
      return;
    case F::B:
      return;
    default:
      return;
  }
}
