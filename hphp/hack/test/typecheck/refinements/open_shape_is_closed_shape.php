<?hh

function test_shape(shape(...) $x): void {
  if ($x is shape('x' => string)) {
    hh_show($x); // shape('x' => string)
  }
}
