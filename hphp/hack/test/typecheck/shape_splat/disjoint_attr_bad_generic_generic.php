<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

<<__DisjointShapeSplat>>
newtype Bad<T1 as shape(...), T2 as shape(...)> = shape(...T1, ...T2);
