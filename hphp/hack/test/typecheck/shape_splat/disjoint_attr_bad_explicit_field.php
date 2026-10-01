<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type A = shape('x' => int);

<<__DisjointShapeSplat>>
type Bad = shape(...A, 'x' => bool);
