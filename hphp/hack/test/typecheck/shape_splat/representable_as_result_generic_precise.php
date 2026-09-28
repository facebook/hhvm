<?hh
<<file:__EnableUnstableFeatures(
  'representable_as',
  'shape_splat_concrete',
  'shape_splat_type_parameters',
)>>

function takes(
  \HH\Runtime\RepresentableAs<dict<string, arraykey>> $_,
): void {}

function test<T as shape('name' => string)>(
  shape(...T, 'id' => int) $shape,
): void {
  takes($shape);
}
