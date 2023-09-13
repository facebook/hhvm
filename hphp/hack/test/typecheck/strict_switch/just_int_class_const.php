<?hh

class C {
  const int X = 1;
}

// class consts are not integer literals but should be bucketed in the same way
function just_int_class_const(int $x) : void {
  switch ($x) {
    case C::X:
      return;
    default:
      return;
  }
}

enum E: int as int {
  A = 0;
  B = 1;
}

enum F: int as int {
  A = 0;
  B = 1;
}

function redundant_class_const(int $x): void {
  switch ($x) {
    case E::A:
      return;
    case E::B:
      return;
    case E::B:
      return;
    default:
      return;
  }
}

function multiple_class_const(int $x): void {
  switch ($x) {
    // We don't store enum values in the typed AST and so cannot see that
    // the cases are redundant
    case E::A:
      return;
    case E::A:
      return;
    case F::B:
      return;
    case F::A:
      return;
    case E::B:
      return;
    default:
      return;
  }
}

enum G: int {
  A = 0;
}


function not_transparent_class_const(int $x): void {
  switch ($x) {
    case G::A:
      return;
    default:
      return;
  }
}
