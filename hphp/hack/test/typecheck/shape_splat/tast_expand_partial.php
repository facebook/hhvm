<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// tast_expand with Shape_splat where not all elements are Shape_simple
// (exercises the else branch that preserves Shape_splat)
function preserve_splat<T as shape(...)>(
  shape(...T, 'x' => int) $s,
): void {
  // Inside the body, T is still abstract — tast_expand cannot normalize
  hh_show($s);
}
