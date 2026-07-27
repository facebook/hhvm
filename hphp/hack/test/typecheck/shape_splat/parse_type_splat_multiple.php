<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type A = shape('x' => int);
type B = shape('y' => string);
type C = shape(...A, 'z' => bool, ...B);

function test(C $s): void {
  hh_show($s);
}
