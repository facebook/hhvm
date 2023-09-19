<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
<<__StrictSwitch>>
function mixed_default(mixed $x): void {
  switch ($x) {
    default:
      return;
  }
}
