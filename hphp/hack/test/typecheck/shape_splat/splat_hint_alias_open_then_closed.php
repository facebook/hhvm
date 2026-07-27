<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type TOpenX = shape('x' => int, ...);
type TClosedY = shape('y' => int);

// Open alias on the left, closed alias on the right. Openness on the left is the
// base, so 'x' stays int; the result is open.
function splat_alias_open_then_closed(shape(...TOpenX, ...TClosedY) $s): void {
  hh_expect<shape('x' => int, 'y' => int, ...)>($s);
  hh_expect<int>($s['x']);
  hh_expect<int>($s['y']);
}
