<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
class StringClass {
  const string X = "X";
}

class IntClass {
  const int X = 0;
}

class Arraykey1 {
  const string X = "X";
}

class Arraykey2 {
  const int X = 0;
}

class Arraykey3 {
  const arraykey X = "X";
}

class Arraykey4 {
  const arraykey X = 0;
}

// class consts are not string literals but should be bucketed in the same way
<<__StrictSwitch>>
function arraykey_class_const(arraykey $x) : void {
  switch ($x) {
    case StringClass::X:
      return;
    case IntClass::X:
      return;
    case Arraykey1::X:
      return;
    case Arraykey2::X:
      return;
    case Arraykey3::X:
      return;
    case Arraykey4::X:
      return;
    default:
      return;
  }
}

enum E: arraykey as arraykey {
  A = "A";
  B = "B";
}

<<__StrictSwitch>>
function transparent_arraykey_enum_no_default(arraykey $x): void {
  switch ($x) {
    case E::A:
      return;
    case E::B:
      return;
  }
}

enum G: string {
  A = "A";
}

enum H: int {
  A = 0;
}

<<__StrictSwitch>>
function enum_subtype_arraykey(arraykey $x): void {
  switch ($x) {
    case G::A:
      return;
    case H::A:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function just_arraykey(arraykey $x): void {
  switch ($x) {
    case StringClass::X:
      return;
    case IntClass::X:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function redundant_literal_arraykey(arraykey $x): void {
  switch ($x) {
    case "0":
      return;
    case 0:
      return;
    case "0":
      return;
    case 0:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function not_subtype_arraykey(arraykey $x): void {
  switch ($x) {
    case true:
      return;
    case null:
      return;
    default:
      return;
  }
}
