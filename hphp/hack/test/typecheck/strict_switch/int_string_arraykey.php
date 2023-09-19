<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
class StringClass {
  const string X = "X";
}

class IntClass {
  const int X = 0;
}

// class consts are not string literals but should be bucketed in the same way
<<__StrictSwitch>>
function arraykey_class_const(arraykey $x) : void {
  switch ($x) {
    case StringClass::X:
      return;
    case IntClass::X:
      return;
    default:
      return;
  }
}
