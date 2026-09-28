<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type Left = shape('field' => bool);
type Right = shape(?'field' => string);

function test(): void {
  $f = (shape(...Left, ...Right) $_) ==> null;
}
