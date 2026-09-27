<?hh
<<file:__EnableUnstableFeatures('shape_splat_type_parameters')>>

// `is nonnull` on a type-parameter splat scrutinee: always true, and the
// abstract splat `...T` is preserved exactly (T is not refined away).
function f<T as shape(...)>(shape(...T, 'a' => int) $x): void {
  if ($x is nonnull) {
    hh_expect_equivalent<shape(...T, 'a' => int)>($x);
  }
}
