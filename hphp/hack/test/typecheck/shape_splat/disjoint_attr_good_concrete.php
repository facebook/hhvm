<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type A = shape('a' => int);
type B = shape('b' => string);

<<__DisjointShapeSplat>>
type Good = shape(...A, ...B);

function test(Good $s): void {
  hh_expect<shape('a' => int, 'b' => string)>($s);
}
