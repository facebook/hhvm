<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
<<__StrictSwitch>>
function just_int(int $x): void {
  switch ($x) {
    case 0:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function non_literal(int $x): void {
  $y = 0;

  switch ($x) {
    case $y:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function missing_default(int $x): void {
  switch ($x) {
    case 0:
      return;
  }
}

<<__StrictSwitch>>
function redundant_literal(int $x): void {
  switch ($x) {
    case 0:
      return;
    case 1:
      return;
    case 0:
      return;
    case 1:
      return;
    default:
      return;
  }
}
