<?hh

<<file: __EnableUnstableFeatures('strict_switch')>>

<<__StrictSwitch>>
function test_negative_primitive(mixed $x): void {
  if ($x is bool) {
    return;
  }
  switch ($x) {
    case null:
      return;
  }
}
