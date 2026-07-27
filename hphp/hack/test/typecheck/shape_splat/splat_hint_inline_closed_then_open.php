<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Splat an inline closed shape hint, then an inline OPEN shape hint. The open
// shape on the right may carry any field at mixed, so under rightmost-wins it
// widens the earlier 'x' to mixed.
function splat_inline_closed_then_open(
  shape(...shape('x' => int), ...shape('y' => int, ...)) $s,
): void {
  hh_expect<shape('x' => mixed, 'y' => int, ...)>($s);
  hh_expect<mixed>($s['x']);
  hh_expect<int>($s['y']);
}
