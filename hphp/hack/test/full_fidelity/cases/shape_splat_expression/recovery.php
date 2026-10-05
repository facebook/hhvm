<?hh
<<file:__EnableUnstableFeatures('shape_splat_expression')>>

function test_recovery(mixed $fields): void {
  $_ = shape(, ...$fields);
  $_ = shape(...$fields,, 'x' => 1);
  $_ = shape('x' => 1 ...$fields);
  $_ = shape(...,);
  $_ = shape(_);
}
