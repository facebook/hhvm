<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
<<__StrictSwitch>>
function missing_null(null $x): void {
  switch ($x) {
    case 42:
      return;
  }
}
