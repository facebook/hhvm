<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type A = shape('x' => int);
type B = shape(...A, 'y' => string);
type C = shape(...B, 'z' => bool);

function test(C $s): void {
  hh_expect<shape('x' => int, 'y' => string, 'z' => bool)>($s);
  hh_expect<int>($s['x']);
  hh_expect<string>($s['y']);
  hh_expect<bool>($s['z']);
}
