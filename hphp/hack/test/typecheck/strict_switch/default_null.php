<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
<<__StrictSwitch>>
function default_null(null $x): void {
  switch ($x) {
    default:
      return;
  }
}
