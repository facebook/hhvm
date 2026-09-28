<?hh
<<file:__EnableUnstableFeatures(
  'like_type_hints',
  'shape_splat_concrete',
)>>

type Optional = shape(?'field' => string);
type Bad = shape(...~shape('field' => bool), ...Optional);
