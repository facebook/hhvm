<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'union_intersection_type_hints',
)>>

type Left = shape('field' => bool);
type Right = shape(?'field' => string);

function allowed_parameter(shape(...Left, ...Right) $_): void {}
