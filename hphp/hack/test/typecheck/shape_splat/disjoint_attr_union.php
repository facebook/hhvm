<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'union_intersection_type_hints')>>

// EXPECTED ERROR: one union branch can supply `x`, so the splat is not
// provably disjoint from the explicit field.
<<__DisjointShapeSplat>>
newtype Bad<
  TA as shape('x' => int),
  TB as shape('b' => int),
> = shape(...(TA | TB), 'x' => bool);

// A concrete field in one union branch is a possible overlap.
<<__DisjointShapeSplat>>
newtype BadConcreteBranch<T as shape('b' => int)> =
  shape(...(shape('x' => int) | T), 'x' => bool);

// SOUND ACCEPTANCE CONTROL: neither union branch can supply `x`.
<<__DisjointShapeSplat>>
newtype Good<
  TA as shape('a' => int),
  TB as shape('b' => int),
> = shape(...(TA | TB), 'x' => bool);
