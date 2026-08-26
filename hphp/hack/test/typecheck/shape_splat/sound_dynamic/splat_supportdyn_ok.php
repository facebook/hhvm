<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// A shape splat is a subtype of the corresponding `supportdyn` shape when every
// part supports dynamic: the concrete field type (`int`) and the spread type
// parameter (bounded by `supportdyn<shape(...)>`).
function ok<T as supportdyn<shape(...)>>(
  shape('a' => int, ...T) $s,
): supportdyn<shape('a' => int, ...T)> {
  return $s;
}

function to_dict<T as supportdyn<shape(...)>>(
  shape('a' => int, ...T) $s,
): dynamic {
  return Shapes::toDict($s);
}

function to_array<T as supportdyn<shape(...)>>(
  shape('a' => int, ...T) $s,
): dynamic {
  return Shapes::toArray($s);
}
