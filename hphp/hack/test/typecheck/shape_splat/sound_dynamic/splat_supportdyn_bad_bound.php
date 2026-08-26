<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// The spread type parameter also has to support dynamic. Here `T` is only bounded
// by `shape(...)` (an open shape whose unknown fields are `mixed`), which does not
// support dynamic, so the splat shape is not a subtype of the `supportdyn` shape.
function bad_bound<T as shape(...)>(
  shape('a' => int, ...T) $s,
): supportdyn<shape('a' => int, ...T)> {
  return $s;
}
