<?hh
<<file:__EnableUnstableFeatures('shape_splat_type_parameters')>>

function f<T>(shape(...T) $x): void
where
  T as shape('a' => int, ...),
  T as shape('b' => string, ...) {
  if ($x is shape('a' => string, ...)) {
    hh_expect_equivalent<nothing>($x);
  }
}
