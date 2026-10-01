<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'union_intersection_type_hints')>>

// REGRESSION (previously a false negative): the witness is inhabited and its
// `x` field is supplied by both the intersection splat and the explicit field.
<<__DisjointShapeSplat>>
newtype Bad<TA as shape(...), TB as shape(...)> =
  shape(...(TA & TB), 'x' => int);

type Witness = Bad<shape('x' => int), shape('x' => int)>;

// An optional field is still possibly present and must be rejected.
<<__DisjointShapeSplat>>
newtype BadOptional<
  TA as shape(?'x' => int),
  TB as shape(...),
> = shape(...(TA & TB), 'x' => bool);
