<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Test that rightmost-wins works correctly through chains
type A = shape('x' => int);
type B = shape(...A, 'x' => string);
type C = shape(...B, 'x' => bool);

function test(C $s): void {
  hh_expect<shape('x' => bool)>($s);
  // Final type of 'x' should be bool (rightmost wins at each level)
  hh_expect<bool>($s['x']);
}
