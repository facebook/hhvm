<?hh

enum E: int {
  A = 0;
  B = 1;
}

enum F: int {
  A = 0;
  B = 1;
}

function test_nonexhaustive(E $e): void {
  switch ($e) {
    case E::A:
      return;
  }
}

function test_wrong_enum_nonexhaustive(E $e): void {
  switch ($e) {
    case F::A:
      return;
    case F::B:
      return;
  }
}
