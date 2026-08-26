<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters', 'shape_splat_expression')>>

function needs_int_suffix<T as shape(...)>(
  shape(...T, 'suffix' => int) $s,
): void {}

function test(): void {
  needs_int_suffix(shape('suffix' => 'wrong', 'tail' => true));
}
