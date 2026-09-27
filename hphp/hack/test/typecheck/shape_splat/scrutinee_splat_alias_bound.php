<?hh
<<file:__EnableUnstableFeatures('shape_splat_type_parameters')>>

type Row = shape('p' => bool, ...);

function f<T as Row>(shape(...T, 'a' => int) $x): void {
  if ($x is shape('a' => int)) {
    hh_expect_equivalent<nothing>($x);
  }
}
