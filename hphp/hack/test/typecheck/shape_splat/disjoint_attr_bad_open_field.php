<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type Open = shape('y' => int, ...);

<<__DisjointShapeSplat>>
type Bad = shape(...Open, 'z' => bool);
