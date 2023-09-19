<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
<<__StrictSwitch>>
function mixed_with_bool_case(mixed $x): void {
  switch ($x) {
    case true:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function mixed_only_bool(mixed $x): void {
  switch ($x) {
    case false:
      return;
  }
}
