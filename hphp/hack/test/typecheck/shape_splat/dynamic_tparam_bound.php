<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// How a `dynamic` splat bound normalizes. What that bound then means for
// subtyping is covered by dynamic_bound_corner.php.

// A bare `dynamic` bound is spreadable: it is the open row
// `shape(_ => dynamic)`.
function bare_dynamic_bound<T as dynamic>(shape(...T) $s): void {
  hh_expect_equivalent<T>($s);
}

// A shape bound whose splat contains `dynamic` normalizes to an open row whose
// unknown fields are `dynamic`, merged with the bound's known fields.
function dynamic_unknown_bound<T as shape(...dynamic, 'x' => int)>(
  shape(...T) $s,
): void {
  hh_expect_equivalent<T>($s);
  hh_expect<int>($s['x']);
}
