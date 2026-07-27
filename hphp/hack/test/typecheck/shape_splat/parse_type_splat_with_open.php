<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type TBase = shape('x' => int);
// Splat + trailing ... (open shape) should both parse
type TOpen = shape(...TBase, 'y' => string, ...);

function test(TOpen $s): void {
  hh_show($s);
}
