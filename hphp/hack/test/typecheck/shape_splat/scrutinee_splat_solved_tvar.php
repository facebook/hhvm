<?hh
<<file:__EnableUnstableFeatures(
  'shape_splat_type_parameters',
)>>

function preserve<T as shape(...)>(
  shape(...T, 'a' => int) $x,
): shape(...T, 'a' => int) {
  return $x;
}

function f(shape('p' => bool, 'a' => int, ...) $input): void {
  $x = preserve($input);
  if ($x is shape('a' => int)) {
    hh_expect_equivalent<nothing>($x);
  }
}
