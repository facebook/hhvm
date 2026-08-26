<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete', 'shape_splat_type_parameters')>>

function read_optional_bound<T as shape(?'nickname' => string, ...)>(
  shape(...T, 'x' => int) $s,
): void {
  hh_expect_equivalent<?string>(Shapes::idx($s,'nickname'));
}
