<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>
<<__StrictSwitch>>
function noreturn_case(mixed $x): void {
  if ($x is int) {
    if ($x is string) {
      switch ($x) {
        case 0:
          return;
      }
    }
  }
  return;
}
