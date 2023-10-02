<?hh

<<file: __EnableUnstableFeatures('union_intersection_type_hints', 'strict_switch')>>

enum IntEnumAB:int {
  A = 1;
  B = 2;
}

enum Zero:int {
}

<<__StrictSwitch>>
function just_zero(Zero $x): void {
  switch ($x) {
    default:
      break;
  }
}

<<__StrictSwitch>>
function union_zero((IntEnumAB | Zero) $x): void {
  switch ($x) {
    case IntEnumAB::A:
      return;
    case IntEnumAB::B:
      return;
  }
}
