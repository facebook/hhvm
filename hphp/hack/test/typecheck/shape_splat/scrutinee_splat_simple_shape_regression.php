<?hh

function f(shape('a' => int) $x): void {
  if ($x is shape('a' => string)) {
    hh_expect_equivalent<nothing>($x);
  }
}
