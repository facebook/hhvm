<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// ACCEPT. T is below the bottom row, so no value can inhabit it and the
// subtyping obligation in the body holds vacuously. The second shape bound
// ensures the combined upper bound also contains a non-bottom shape row.
function bottom_upper_intersection<T as shape(...)>(
  shape(...T, 'q' => int) $value,
): shape('x' => string, 'q' => int, ...)
  where T as shape(...nothing), T as shape('x' => int, ...) {
  return $value;
}
