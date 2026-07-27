<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Splat a single inline OPEN shape hint.
function splat_inline_open(shape(...shape('x' => int, ...)) $s): void {
  hh_expect<shape('x' => int, ...)>($s);
  hh_expect<int>($s['x']);
}
