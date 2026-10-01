<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'union_intersection_type_hints')>>

// REGRESSION (previously a false negative): the union is handled, and its
// intersection branch must not hide the overlap demonstrated by the witness.
<<__DisjointShapeSplat>>
newtype Bad<TA as shape(...), TB as shape(...)> =
  shape(...((TA & TB) | shape('a' => int)), 'x' => bool);

type Witness = Bad<shape('x' => int), shape('x' => int)>;
