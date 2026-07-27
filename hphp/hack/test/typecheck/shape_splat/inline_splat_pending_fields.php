<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Multiple pending fields between splats — exercises decl_hint.ml accumulation
type T1 = shape('x' => int);
type T2 = shape('z' => bool);

function multi_pending(
  shape('a' => int, ...T1, 'b' => string, ...T2, 'c' => float) $s,
): void {
  hh_expect<int>($s['a']);
  hh_expect<int>($s['x']);
  hh_expect<string>($s['b']);
  hh_expect<bool>($s['z']);
  hh_expect<float>($s['c']);
}
