<?hh
<<file:__EnableUnstableFeatures('shape_splat_type_parameters')>>

// The scrutinee always has the concrete field 'a', but the closed hint does not
// permit it. No value can satisfy both, so the true branch is `nothing`.
function f<T as shape(...)>(shape(...T, 'a' => int) $x): void {
  if ($x is shape('b' => int)) {
    hh_expect_equivalent<nothing>($x);
  }
}
