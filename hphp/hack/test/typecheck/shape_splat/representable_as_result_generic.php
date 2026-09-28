<?hh
<<file:__EnableUnstableFeatures(
  'representable_as',
  'shape_splat_concrete',
  'shape_splat_type_parameters',
)>>

function takes(
  \HH\Runtime\RepresentableAs<dict<arraykey, mixed>> $_,
): void {}

function test<T as shape(...)>(shape(...T, 'x' => int) $shape): void {
  takes($shape);
}
