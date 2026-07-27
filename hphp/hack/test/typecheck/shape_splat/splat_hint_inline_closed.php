<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Splat a single inline closed shape hint.
function splat_inline_closed(shape(...shape('x' => int)) $s): void {
  hh_expect<shape('x' => int)>($s);
  hh_expect<int>($s['x']);
}
