<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>

enum IntEnumAB:int {
  A = 1;
  B = 2;
}

<<__StrictSwitch>>
function just_enum(IntEnumAB $x): void {
  switch ($x) {
    case IntEnumAB::A:
      return;
    case IntEnumAB::B:
      return;
  }
}

<<__StrictSwitch>>
function redundant_default(IntEnumAB $x): void {
  switch ($x) {
    case IntEnumAB::A:
      return;
    case IntEnumAB::B:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function redundant_label(IntEnumAB $x): void {
  switch ($x) {
    case IntEnumAB::A:
      return;
    case IntEnumAB::A:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function opaque(int $x): void {
  switch ($x) {
    case IntEnumAB::A:
      return;
    case IntEnumAB::B:
      return;
    default:
      return;
  }
}
