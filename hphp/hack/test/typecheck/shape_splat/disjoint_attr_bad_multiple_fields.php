<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type A = shape('x' => int, 'y' => int);
type B = shape('x' => string, 'y' => string);
type C = shape('x' => bool);

<<__DisjointShapeSplat>>
type Bad = shape(...A, ...B, ...C);
