<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

function f(shape('a' => int, ...nothing) $x): void {
  if ($x is shape('a' => int)) {
    hh_expect_equivalent<nothing>($x);
  } else {
    hh_expect_equivalent<nothing>($x);
  }
}
