<?hh
<<file:__EnableUnstableFeatures(
  'representable_as',
  'shape_splat_concrete',
  'shape_splat_type_parameters',
)>>

function takes_string_keys(
  \HH\Runtime\RepresentableAs<dict<string, mixed>> $_,
): void {}

function takes_int_values(
  \HH\Runtime\RepresentableAs<dict<arraykey, int>> $_,
): void {}

function test<T as shape(...)>(shape(...T, 'x' => int) $shape): void {
  takes_string_keys($shape);
  takes_int_values($shape);
}
