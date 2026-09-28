<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

type Left = shape('field' => bool);
type Right = shape(?'field' => string);

function masked_nested_union(): shape(
  'outer' => shape(...Left, ...Right),
  ...shape('outer' => int),
) {
  throw new Exception();
}
