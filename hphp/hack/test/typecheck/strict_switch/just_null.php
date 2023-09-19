<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
<<__StrictSwitch>>
function just_null(null $x): void {
  switch ($x) {
    case null:
      return;
  }
}

<<__StrictSwitch>>
function non_literal(null $x): void {
  $y = null;

  switch ($x) {
    case $y:
      return;
  }
}

<<__StrictSwitch>>
function redundant_default(null $x): void {
  switch ($x) {
    case null:
      return;
    default:
      return;
  }
}
