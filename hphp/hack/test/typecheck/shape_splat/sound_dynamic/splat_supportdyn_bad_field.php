<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

class NonSdt {} // does not support dynamic (everything_sdt=false)

// A concrete field that does not support dynamic makes the splat shape not a
// subtype of the corresponding `supportdyn` shape. The splat subtyping path must
// reject it (the whole splat is decomposed under dynamic-aware subtyping).
function bad_field<T as supportdyn<shape(...)>>(
  shape('a' => NonSdt, ...T) $s,
): supportdyn<shape('a' => NonSdt, ...T)> {
  return $s;
}
