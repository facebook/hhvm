<?hh
<<file:__EnableUnstableFeatures(
  'shape_and_tuple_destructuring',
  'shape_field_punning',
  'shape_splat_expression',
)>>

function test(mixed $source): void {
  shape('x' => $x, ...) = $source;
  foreach ($source as shape(?$value, ...)) {}

  shape(...1) = $source;
  shape('x' => $x, ...$source) = $source;
  foreach ($source as shape(...2)) {}
}
