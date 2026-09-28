<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type Left = shape('field' => bool);
type Right = shape(?'field' => string);

function bad_return(): (function(shape(...Left, ...Right)): void) {
  throw new Exception();
}
