<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// Two type parameters with disjoint CLOSED bounds cannot overlap: any T1
// supplies only 'a' and any T2 only 'b', with no unknown rows.
<<__DisjointShapeSplat>>
newtype Good<T1 as shape('a' => int), T2 as shape('b' => int)> =
  shape(...T1, ...T2);
