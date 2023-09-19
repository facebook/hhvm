<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
<<__StrictSwitch>>
function double_null(null $x): void {
  switch ($x) {
    case null:
      return;
    case null:
      return;
  }
}
