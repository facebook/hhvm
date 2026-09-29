<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>
type XIsInt = shape(
  'anchor' => int,
  'x' => int,
);
// CONTROL: REJECT both before and after the fix.
//
// TRight does not constrain `x`, so neither does TLeft. Consequently an `x`
// supplied by TLeft cannot safely be returned as a string.
function acyclic_bounds_leave_x_unconstrained<
  TLeft,
  TRight as shape(...),
>(shape(...TLeft, 'q' => int) $value): shape(
  'anchor' => int,
  ?'x' => string,
  'q' => int,
)
where
  TLeft as shape(...TRight, 'anchor' => int) {
  return $value;
}
// REGRESSION: this must be rejected for exactly the same reason as the
// acyclic control. Adding the reverse dependency does not constrain `x`; it
// merely creates a cycle between TLeft and TRight.
//
// Before the fix, corner search broke that cycle and interpreted the missing
// assignment as proof that `x` was absent. It therefore accepted this return.
function cyclic_bounds_also_leave_x_unconstrained<
  TLeft,
  TRight,
>(shape(...TLeft, 'q' => int) $value): shape(
  'anchor' => int,
  ?'x' => string,
  'q' => int,
)
where
  TLeft as shape(...TRight, 'anchor' => int),
  TRight as shape(...TLeft, 'anchor' => int) {
  return $value;
}
// The cyclic constraints are satisfiable. Choosing XIsInt for both parameters
// produces a value with `x: int`, demonstrating why the return above is false.
function concrete_cyclic_witness(
  shape('anchor' => int, 'x' => int, 'q' => int) $value,
): shape('anchor' => int, ?'x' => string, 'q' => int) {
  return cyclic_bounds_also_leave_x_unconstrained<XIsInt, XIsInt>($value);
}
