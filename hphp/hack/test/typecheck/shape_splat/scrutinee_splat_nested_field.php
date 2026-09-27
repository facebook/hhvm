<?hh
<<file:__EnableUnstableFeatures('shape_splat_type_parameters')>>

// Splat nested inside a field value: refining the projected field keeps `...T`
// exactly.
function f<T as shape(...)>(shape('k' => shape(...T, 'a' => int)) $x): void {
  if ($x['k'] is nonnull) {
    hh_expect_equivalent<shape(...T, 'a' => int)>($x['k']);
  }
}
