<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type TBase = shape('x' => int, 'y' => string);
type TExtended = shape(...TBase, 'z' => bool);

// Subtyping should fail: TExtended has 'x' => int but target expects 'x' => string
function takes_wrong(shape('x' => string, 'y' => string, 'z' => bool) $s): void {}

function test(TExtended $s): void {
  takes_wrong($s);
}
