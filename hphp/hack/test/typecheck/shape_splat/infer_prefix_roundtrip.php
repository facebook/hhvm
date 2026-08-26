<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

// `keep_prefixed` preserves the required `prefix` field and infers the
// remaining `tail` field.
function keep_prefixed<T as shape(...)>(
  shape('prefix' => int, ...T) $s,
): shape('prefix' => int, ...T) {
  return $s;
}

function test(): void {
  $r = keep_prefixed(shape('prefix' => 1, 'tail' => true));
  hh_expect<shape('prefix' => int, 'tail' => bool)>($r);
}
