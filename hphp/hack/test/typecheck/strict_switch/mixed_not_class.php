<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
class C {}

<<__StrictSwitch>>
function mixed_not_class1(mixed $x): void {
  if ($x is C) {
    return;
  }

  switch ($x) {
    case null:
      return;
  }
}

<<__StrictSwitch>>
function mixed_not_class2(mixed $x): void {
  if ($x is C) {
    return;
  }

  switch ($x) {
    case null:
      return;
    default:
      return;
  }
}
