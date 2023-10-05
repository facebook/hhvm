<?hh

<<file: __EnableUnstableFeatures('union_intersection_type_hints', 'strict_switch')>>

<<__StrictSwitch>>
function just_dynamic(dynamic $x): void {
  switch ($x) {
    default:
      return;
  }
}

<<__StrictSwitch>>
function just_dynamic_no_default(dynamic $x): void {
  switch ($x) {
    case 42:
      return;
  }
}

// This is ~null and so only requires a null case
<<__StrictSwitch>>
function nullable_dynamic(?dynamic $x): void {
  switch ($x) {
    case null:
      return;
  }
}

enum IntEnumAB:int {
  A = 1;
  B = 2;
}

<<__StrictSwitch>>
function like_enum((dynamic | IntEnumAB) $x): void {
  switch ($x) {
    case IntEnumAB::A:
      return;
    case IntEnumAB::B:
      return;
  }
}

<<__StrictSwitch>>
function like_int_enum(((int & dynamic) | IntEnumAB) $x): void {
  switch ($x) {
    case IntEnumAB::A:
      return;
    case IntEnumAB::B:
      return;
  }
}

<<__StrictSwitch>>
function intersect_arraykey_like_enum((arraykey & (dynamic | IntEnumAB)) $x): void {
  switch ($x) {
    case IntEnumAB::A:
      return;
    case IntEnumAB::B:
      return;
  }
}
