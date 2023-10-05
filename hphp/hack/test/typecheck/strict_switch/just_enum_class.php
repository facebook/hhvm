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

enum class MixedEnumClass: mixed {
  IntEnumAB LabelA = IntEnumAB::A;
  IntEnumCD LabelC = IntEnumCD::C;
  string LabelString1 = "string1";
  int LabelInt = 1;
  int LabelInt2 = 1;
  bool LabelBool = true;
}

<<__StrictSwitch>>
function enum_class(MixedEnumClass $x): void {
  switch ($x) {
    case MixedEnumClass::LabelC:
      return;
    default:
      return;
  }
}

/* This is not supported because we would rather encourage the user to write
   literals (true/false, or null for nullables/null) because we cannot see
   values of class constants in the typechecker. */
<<__StrictSwitch>>
function enum_class_memberof_nonarraykey_subtype(bool $x): void {
  switch ($x) {
    case MixedEnumClass::LabelBool:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function enum_class_memberof_arraykey_subtype(string $x): void {
  switch ($x) {
    case MixedEnumClass::LabelString1:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function enum_class_memberof_wrong(\HH\MemberOf<MixedEnumClass, string> $x): void {
  switch ($x) {
    case MixedEnumClass::LabelInt:
      return;
  }
}

<<__StrictSwitch>>
function enum_class_memberof_two_int(\HH\MemberOf<MixedEnumClass, int> $x): void {
  switch ($x) {
    case MixedEnumClass::LabelInt:
      return;
    case MixedEnumClass::LabelInt2:
      return;
  }
}

<<__StrictSwitch>>
function enum_class_union_memberof((\HH\MemberOf<MixedEnumClass, IntEnumAB> | \HH\MemberOf<MixedEnumClass, IntEnumCD>) $x): void {
  switch ($x) {
    case MixedEnumClass::LabelA:
      return;
  }
}

enum class ExtendedEnumClass: mixed extends MixedEnumClass {
  string LabelString2 = "string2";
}

<<__StrictSwitch>>
function extended_enum_class_memberof(\HH\MemberOf<ExtendedEnumClass, string> $x): void {
  switch ($x) {
    case ExtendedEnumClass::LabelString1:
      return;
    case ExtendedEnumClass::LabelString2:
      return;
  }
}
