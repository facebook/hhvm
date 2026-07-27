<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// Inline nested splat where the inner shape splats an abstract generic, so the
// inner splat cannot fully reduce. The residual must still be FLAT: the inner
// shape's leading simple fields ('y') merge with the outer's ('x') into one
// simple shape, followed by ...T -- NOT a Shape_splat nested inside another.
function nested_generic<T as shape(...)>(
  shape('x' => int, ...shape('y' => bool, ...T)) $s,
): void {
  hh_show($s);
}
