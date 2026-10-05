<?hh
<<file:__EnableUnstableFeatures(
  'shape_field_punning',
  'shape_splat_expression',
)>>

function test(mixed $fields): void {
  $_ = shape(...$fields);
}

function test_multiple_splats(mixed $a, mixed $b, mixed $c): void {
  $_ = shape(...$a, ...$b, ...$c);
}

function test_field_then_splat(mixed $tail): void {
  $_ = shape('x' => 2, 'y' => 'hi', ...$tail);
}

function test_interleaved(mixed $middle): void {
  $_ = shape('a' => 1, ...$middle, 'b' => 'hi');
}

function test_trailing_comma(mixed $fields): void {
  $_ = shape(...$fields,);
  $_ = shape('x' => 1,);
}

function test_expression_operands(
  mixed $factory,
  bool $condition,
  mixed $a,
  mixed $b,
): void {
  $_ = shape(...$factory($a, $b));
  $_ = shape(...$condition ? $a : $b, 'x' => 1);
}

function test_punned_fields(mixed $first, mixed $middle, mixed $last): void {
  $_ = shape($first, ...$middle, $last);
}
