<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

<<__DisjointShapeSplat>>
type Bad = shape(...shape(...), ...shape(...));
