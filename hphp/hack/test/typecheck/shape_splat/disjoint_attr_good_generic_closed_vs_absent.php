<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// T1 can only ever supply 'a', and T2's bound pins 'a' absent, so the two are
// disjoint even though T2's row is open. Proving this needs both bounds read as
// label sets; comparing them pairwise as opaque "unknown" sources cannot.
<<__DisjointShapeSplat>>
newtype Good<T1 as shape('a' => int), T2 as shape(?'a' => nothing, ...)> =
  shape(...T1, ...T2);
