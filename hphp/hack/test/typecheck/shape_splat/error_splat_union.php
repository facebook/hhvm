<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Error: non-shape type is not a valid splat element
type NotAShape = int;

type Bad = shape(...NotAShape, 'x' => int);

function test(Bad $b): void {}
