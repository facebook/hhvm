<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
<<__StrictSwitch>>
function mixed_no_default(mixed $x): void {
  switch ($x) {
    case vec[]:
      return;
  }
}
