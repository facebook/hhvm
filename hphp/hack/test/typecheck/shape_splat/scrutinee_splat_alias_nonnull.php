<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Concrete-alias splat scrutinee: the alias is normalized into a plain shape,
// so `is nonnull` is always true and pins the merged shape exactly.
type MyRow = shape('b' => string);

function f(shape(...MyRow, 'a' => int) $x): void {
  if ($x is nonnull) {
    hh_expect_equivalent<shape('a' => int, 'b' => string)>($x);
  }
}
