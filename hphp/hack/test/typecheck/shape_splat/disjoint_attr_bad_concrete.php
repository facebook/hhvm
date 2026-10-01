<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type A = shape('x' => int);
type B = shape('x' => string);

<<__DisjointShapeSplat>>
type Bad = shape(...A, ...B);

<<__DisjointShapeSplat>>
type BadOptionalRequired =
  shape(...shape(?'x' => int), ...shape('x' => string));

<<__DisjointShapeSplat>>
type BadOptionalOptional =
  shape(...shape(?'x' => int), ...shape(?'x' => string));

<<__DisjointShapeSplat>>
type GoodAbsentRequired =
  shape(...shape(?'x' => nothing), ...shape('x' => string));
