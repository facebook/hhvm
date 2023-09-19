<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
<<__StrictSwitch>>
function mixed_with_null_case(mixed $x): void {
  switch ($x) {
    case null:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function mixed_only_null(mixed $x): void {
  switch ($x) {
    case null:
      return;
  }
}
