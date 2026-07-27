<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// A leading field then an inline OPEN shape hint splat. The trailing open shape
// widens the earlier 'z' to mixed.
function splat_field_then_inline_open(
  shape('z' => bool, ...shape('x' => int, ...)) $s,
): void {
  hh_expect<shape('x' => int, 'z' => mixed, ...)>($s);
  hh_expect<int>($s['x']);
  hh_expect<mixed>($s['z']);
}
