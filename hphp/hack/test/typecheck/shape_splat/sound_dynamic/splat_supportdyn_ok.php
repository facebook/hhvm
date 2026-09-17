<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
  'union_intersection_type_hints',
)>>

// A shape splat is a subtype of the corresponding `supportdyn` shape when every
// part supports dynamic: the concrete field type (`int`) and the spread type
// parameter (bounded by `supportdyn<shape(...)>`).
function ok<T as supportdyn<shape(...)>>(
  shape('a' => int, ...T) $s,
): supportdyn<shape('a' => int, ...T)> {
  return $s;
}

function distributed_union<T1 as shape(...), T2 as shape(...)>(
  supportdyn<shape(...(shape(...T1) | shape(...T2)))> $s,
): supportdyn<shape(...)> {
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
