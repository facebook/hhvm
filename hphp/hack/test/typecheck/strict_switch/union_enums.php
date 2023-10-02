<?hh

<<file: __EnableUnstableFeatures('union_intersection_type_hints', 'strict_switch')>>

enum IntEnumAB:int {
  A = 1;
  B = 2;
}

enum IntEnumCD:int {
  C = 1;
  D = 2;
}

<<__StrictSwitch>>
function union_enum((IntEnumAB | IntEnumCD) $x): void {
  switch ($x) {
    case IntEnumAB::A:
      return;
    case IntEnumAB::B:
      return;
    case IntEnumCD::C:
    case IntEnumCD::D:
      return;
  }
}

<<__StrictSwitch>>
function missing_two((IntEnumAB | IntEnumCD) $x): void {
  switch ($x) {
    case IntEnumAB::A:
      return;
    case IntEnumCD::D:
      return;
  }
}

<<__StrictSwitch>>
function union_negation((IntEnumAB | IntEnumCD) $x): void {
  if ($x is IntEnumCD) {
    return;
  }

  // Ideally this would be allowed, but the type simplification does not
  // support this and supporting it in the strict switch implementation
  // would require restructuring it.
  switch ($x) {
    case IntEnumAB::A:
      return;
    case IntEnumAB::B:
      return;
  }
}
