<?hh
<<file:__EnableUnstableFeatures('shape_splat_type_parameters')>>

// The splat type parameter has a shape bound *and* a non-shape bound (int).
// Their intersection is `nothing` (a value can't be both a shape and an int), so
// `T` — and the scrutinee — is uninhabited. All upper bounds (not just the shape
// ones) are intersected, so this bottom is preserved: both branches are `nothing`.
function f<T as shape('a' => int) as int>(shape(...T, 'c' => bool) $x): void {
  if ($x is shape('c' => bool)) {
    hh_expect_equivalent<nothing>($x);
  } else {
    hh_expect_equivalent<nothing>($x);
  }
}
