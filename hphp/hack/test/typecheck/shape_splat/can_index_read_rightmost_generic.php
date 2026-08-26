<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

function read_rightmost_generic<T as shape(...)>(
  shape('id' => int, ...T) $s,
): void {
  hh_expect_equivalent<mixed>($s['id']);
}
