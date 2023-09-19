<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
<<__StrictSwitch>>
function just_nullable_bool(?bool $x): void {
  switch ($x) {
    case null:
      return;
    case true:
      return;
    case false:
      return;
  }
}

<<__StrictSwitch>>
function default_true(?bool $x): void {
  switch ($x) {
    case null:
      return;
    case false:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function missing_false(?bool $x): void {
  switch ($x) {
    case null:
      return;
    case true:
      return;
  }
}
