<?hh
<<file:__EnableUnstableFeatures('shape_splat_type_parameters')>>

// `is int` on a type-parameter splat scrutinee: a shape is disjoint from int,
// so the true branch is `nothing` and the false branch keeps `...T` exactly.
function f<T as shape(...)>(shape(...T, 'a' => int) $x): void {
  if ($x is int) {
    hh_expect_equivalent<nothing>($x);
  } else {
    hh_expect_equivalent<shape(...T, 'a' => int)>($x);
  }
}
