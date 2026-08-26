<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

// Good variant: `T`'s bound proves `x` absent (`?'x' => nothing`), so removing
// the masking concrete `'x'` field is sound and allowed. Inferring `T` from the
// argument, the concrete `'x' => int` masks the spread var; after solving, `T`
// should be `shape('a' => bool)` with `x` ABSENT, not a required `'x' => nothing`
// (which would make the shape uninhabited).
// See infer_masked_required_nothing_bad.php for the unsound open-bound variant.
function producer<T as shape(absent 'x', ...)>(
  shape(...T, 'x' => int) $s,
): shape(...T) {
  Shapes::removeKey(inout $s, 'x');
  return $s;
}

function test(): void {
  $arg = shape('a' => true, 'x' => 42);
  $r = producer($arg);
  hh_expect_equivalent<shape('a' => bool, absent 'x')>($r);
}
