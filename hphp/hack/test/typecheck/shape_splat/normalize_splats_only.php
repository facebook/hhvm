<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type A = shape('x' => int);
type B = shape('y' => string);
// No inline fields, just splats
type C = shape(...A, ...B);

function test(C $s): void {
  hh_expect<shape('x' => int, 'y' => string)>($s);
  hh_expect<int>($s['x']);
  hh_expect<string>($s['y']);
}
