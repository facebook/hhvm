<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Splat an inline OPEN shape hint, then an inline closed shape hint. Openness
// on the left (the base) does not pollute later fields, so 'x' stays int. The
// result is open because openness is infectious.
function splat_inline_open_then_closed(
  shape(...shape('x' => int, ...), ...shape('y' => int)) $s,
): void {
  hh_expect<shape('x' => int, 'y' => int, ...)>($s);
  hh_expect<int>($s['x']);
  hh_expect<int>($s['y']);
}
