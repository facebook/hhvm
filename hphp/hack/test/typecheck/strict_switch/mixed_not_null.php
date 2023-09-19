<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
<<__StrictSwitch>>
function mixed_not_null(mixed $x): void {
  if ($x is null) {
    return;
  }

  switch ($x) {
    case null:
      return;
    default:
      return;
  }
}
