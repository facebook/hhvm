<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'union_intersection_type_hints',
)>>

type Left = shape('field' => bool);
type Right = shape(?'field' => string);

abstract class C {
  abstract public function allowed_return(): shape(...Left, ...Right);
}
