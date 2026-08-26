<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// Inside a function body, Shape_splat{Tgeneric} <: Shape_simple
// This exercises the catch-all bound resolution path
function body_subtype<T as shape('id' => int, ...)>(
  shape(...T, 'name' => string) $s,
): shape('id' => int, 'name' => string, ...) {
  // $s has type shape(...T, 'name' => string) where T as shape('id' => int, ...)
  // This should be a subtype of shape('id' => int, 'name' => string, ...)
  return $s;
}
