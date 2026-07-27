<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type TClosedX = shape('x' => int);
type TOpenY = shape('y' => int, ...);

// Closed alias on the left, open alias on the right. The open shape on the right
// widens the earlier 'x' to mixed.
function splat_alias_closed_then_open(shape(...TClosedX, ...TOpenY) $s): void {
  hh_expect<shape('x' => mixed, 'y' => int, ...)>($s);
  hh_expect<mixed>($s['x']);
  hh_expect<int>($s['y']);
}
