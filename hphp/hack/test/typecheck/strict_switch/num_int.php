<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
<<__StrictSwitch>>
function just_num(num $x): void {
  switch ($x) {
    case 0:
      return;
    case 0.5:
      return;
    default:
      return;
  }
}

<<__StrictSwitch>>
function missing_num(num $x): void {
  switch ($x) {
    case null:
      return;
  }
}

<<__StrictSwitch>>
function redundant_num(num $x): void {
  switch ($x) {
    case 0:
      return;
    case true:
      return;
    default:
      return;
  }
}
