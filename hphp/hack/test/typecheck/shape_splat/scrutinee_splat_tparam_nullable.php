<?hh
<<file:__EnableUnstableFeatures('shape_splat_type_parameters')>>

// null / nonnull refinement of a nullable type-parameter splat scrutinee:
// the null branch is exactly `null`, the other keeps `...T` exactly.
function f<T as shape(...)>(?shape(...T, 'a' => int) $x): void {
  if ($x is null) {
    hh_expect_equivalent<null>($x);
  } else {
    hh_expect_equivalent<shape(...T, 'a' => int)>($x);
  }
}
