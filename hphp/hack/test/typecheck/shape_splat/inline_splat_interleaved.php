<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Interleaved fields and splats in inline function param type
type A = shape('a' => int);
type B = shape('c' => bool);

function interleaved(shape(...A, 'b' => string, ...B) $s): void {
  hh_expect<int>($s['a']);
  hh_expect<string>($s['b']);
  hh_expect<bool>($s['c']);
}

function test_interleaved(): void {
  $s = shape('a' => 1, 'b' => 'hello', 'c' => true);
  interleaved($s);
}
