<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// `dynamic` composes with a residual type-parameter splat: the type parameter
// cannot be collapsed, so it stays as a splat element while the `dynamic` still
// contributes its open row to the merged concrete portion.

function test<T as shape(...)>(shape(...T, ...dynamic, 'x' => int) $s): void {
  hh_show($s);
}
