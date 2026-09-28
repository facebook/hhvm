<?hh
<<file:__EnableUnstableFeatures(
  'representable_as',
  'shape_splat_concrete',
  'shape_splat_type_parameters',
  'union_intersection_type_hints',
)>>

function test<
  T as (
    \HH\Runtime\RepresentableAs<shape('x' => int)> &
    shape('y' => string)
  ),
>(shape(...T, 'z' => bool) $_): void {}

function valid(
  shape(
    ...(shape(...) & \HH\Runtime\RepresentableAs<shape('x' => int)>),
    'z' => bool,
  ) $_,
): void {}
