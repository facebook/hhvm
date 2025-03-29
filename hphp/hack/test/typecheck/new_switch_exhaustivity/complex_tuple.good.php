<?hh

enum E: int {
  A = 1;
  B = 2;
}

enum F: int {
  A = 3;
  B = 4;
}

enum G: int {
  A = 5;
  B = 6;
}

enum H: int {
  A = 7;
  B = 8;
}

function main(((E | F), (G & H), ()) $x): void {
  switch ($x) {
    case tuple(E::A, H::A, tuple()):
      break;
    case tuple(E::A, H::B, tuple()):
      break;
    case tuple(E::B, H::A, tuple()):
      break;
    case tuple(E::B, H::B, tuple()):
      break;
    case tuple(F::A, H::A, tuple()):
      break;
    case tuple(F::A, H::B, tuple()):
      break;
    case tuple(F::B, H::A, tuple()):
      break;
    case tuple(F::B, H::B, tuple()):
      break;
  }
}
