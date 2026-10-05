<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_concrete',
  'shape_splat_type_parameters',
  'shape_splat_expression',
  'union_intersection_type_hints',
)>>

// Every possible value of a splat operand must be a shape.

// A plain non-shape (int) operand.
function splat_int(int $n): void {
  $s = shape(...$n, 'x' => 1);
  hh_show($s);
}

function splat_nullable(?shape('a' => int) $opt): void {
  $s = shape(...$opt, 'x' => 1);
  hh_show($s);
  hh_expect<int>($s['a']);
}

function splat_unbounded_generic<T>(T $value): void {
  $s = shape(...$value);
}

function splat_non_shape_bound<T as int>(T $value): void {
  $s = shape(...$value);
}

function splat_union_with_non_shape((shape('a' => int) | int) $value): void {
  $s = shape(...$value);
}

function splat_shape_intersection<
  T1 as shape(...),
  T2 as shape(...),
>((T1 & T2) $value): void {
  $s = shape(...$value);
}
