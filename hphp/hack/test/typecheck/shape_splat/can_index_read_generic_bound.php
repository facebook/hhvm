<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

function read_generic_bound<T as shape('id' => int, ...)>(
  shape(...T, 'name' => string) $s,
): void {
  hh_expect<int>($s['id']);
}
