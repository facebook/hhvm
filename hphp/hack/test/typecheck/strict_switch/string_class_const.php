<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
class C {
  const string X = "X";
}

// class consts are not string literals but should be bucketed in the same way
<<__StrictSwitch>>
function string_class_const(string $x) : void {
  switch ($x) {
    case C::X:
      return;
    default:
      return;
  }
}

enum E: string as string {
  A = "A";
  B = "B";
}

enum F: string as string {
  A = "A";
  B = "B";
}

<<__StrictSwitch>>
function redundant_class_const(string $x): void {
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

<<__StrictSwitch>>
function multiple_class_const(string $x): void {
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

enum G: string {
  A = "A";
}


<<__StrictSwitch>>
function not_transparent_class_const(string $x): void {
  switch ($x) {
    case G::A:
      return;
    default:
      return;
  }
}
