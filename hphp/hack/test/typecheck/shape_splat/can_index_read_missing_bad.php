<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

function read_missing<T as shape(...)>(
  shape(...T, 'name' => string) $s,
): void {
  $s['missing'];
}
