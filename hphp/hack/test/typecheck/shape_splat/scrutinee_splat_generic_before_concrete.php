<?hh
<<file:__EnableUnstableFeatures('shape_splat_type_parameters')>>

function f<T as shape(...)>(shape(...T, 'a' => int) $x): void {
  if ($x is shape('a' => string, ...)) {
    hh_expect_equivalent<nothing>($x);
  }
}
