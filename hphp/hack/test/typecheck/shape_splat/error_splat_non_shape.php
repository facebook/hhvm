<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type TNotAShape = int;
// Splatting a non-shape type should produce an error
type TBad = shape(...TNotAShape, 'x' => bool);

function test(TBad $s): void {}
