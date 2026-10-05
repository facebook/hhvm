<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// The bounds prove `x`/`y` absent (`absent ...`), so removing the masking
// concrete fields is sound and `removeKey` is allowed.
function test<T1 as shape(absent 'x', ...), T2 as shape(absent 'y', ...)>(
  shape(...T1, 'x' => int) $has_x,
  shape(...T2, 'y' => int) $has_y,
): shape(...T1, ...T2) {
  Shapes::removeKey(inout $has_x, 'x');
  Shapes::removeKey(inout $has_y, 'y');
  return shape(...$has_x, ...$has_y);
}

function call(
  shape('a' => bool, 'b' => bool, 'x' => int) $has_x,
  shape(?'b' => int, 'c' => bool, 'y' => int) $has_y,
): void {
  $out = test($has_x, $has_y);
  // NOTE: kept as hh_show — the merged `shape(...T1, ...T2)` result prints 'b' as
  // required but subtypes as optional, so no hh_expect faithfully captures it.
  hh_show($out);
}
