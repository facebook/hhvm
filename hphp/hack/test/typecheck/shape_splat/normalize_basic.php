<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type TBase = shape('x' => int, 'y' => string);
type TExtended = shape(...TBase, 'z' => bool);

function test(TExtended $s): void {
  hh_expect<shape('x' => int, 'y' => string, 'z' => bool)>($s);
  $s['x'];
  $s['y'];
  $s['z'];
}
