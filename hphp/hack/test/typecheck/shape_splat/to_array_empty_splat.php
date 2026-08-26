<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Shapes::toArray on a splat that merges to the empty shape.
function f(shape(...shape()) $s): void {
  hh_expect_equivalent<dict<nothing, nothing>>(Shapes::toArray($s));
}
