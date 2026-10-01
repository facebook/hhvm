<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

<<__DisjointShapeSplat>>
newtype Good<T as shape(?'id' => nothing, ...)> =
  shape(...T, 'id' => int);
