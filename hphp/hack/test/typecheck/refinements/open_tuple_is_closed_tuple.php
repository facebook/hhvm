<?hh

function test_variadic((mixed...) $x): void {
  if ($x is (int, string)) {
    hh_show($x);
  }
}
