<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// `is`/`as` on a shape splat is statically banned: HHVM's is/as matcher only
// understands a *resolved* shape's `fields`, not a splat's `splat_elem_types`,
// and an operand like an erased type parameter cannot be resolved at runtime.
// Both a concrete-alias splat and a type-parameter splat are rejected, for both
// `is` and `as`.

type TBase = shape('x' => int);

function concrete(mixed $m): void {
  if ($m is shape(...TBase, 'b' => int)) {}
  $m as shape(...TBase, 'b' => int);
}

function typaram<T as shape(...)>(mixed $m): void {
  if ($m is shape(...T)) {}
  $m as shape(...T);
}
