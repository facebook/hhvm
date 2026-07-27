<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type TClosedAlias = shape('q' => int);

// Splat a closed alias then an inline OPEN shape hint. The trailing open shape
// widens the earlier 'q' to mixed.
function splat_alias_then_inline_open(
  shape(...TClosedAlias, ...shape('x' => int, ...)) $s,
): void {
  hh_expect<shape('q' => mixed, 'x' => int, ...)>($s);
  hh_expect<mixed>($s['q']);
  hh_expect<int>($s['x']);
}
