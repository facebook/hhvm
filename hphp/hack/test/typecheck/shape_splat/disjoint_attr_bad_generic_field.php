<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

<<__DisjointShapeSplat>>
newtype Bad<T as shape(...)> = shape(...T, 'id' => int);
