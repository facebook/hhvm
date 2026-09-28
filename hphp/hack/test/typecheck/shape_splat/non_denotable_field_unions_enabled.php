<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'union_intersection_type_hints',
)>>

type Left = shape('field' => bool);
type Right = shape(?'field' => string);

// This union is writable while union/intersection type hints are enabled.
type Allowed = shape(...Left, ...Right);
