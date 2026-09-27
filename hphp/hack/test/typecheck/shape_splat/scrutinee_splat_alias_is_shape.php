<?hh
<<file:__EnableUnstableFeatures('shape_splat_concrete')>>

// Concrete-alias splat scrutinee normalizes to a plain shape, so refinement
// matches ordinary closed-shape behavior: intersecting with a shape that lacks
// the known field 'b' is empty (`nothing`); the false branch keeps the merged
// shape exactly.
type MyRow = shape('b' => string);

function f(shape(...MyRow, 'a' => int) $x): void {
  if ($x is shape('a' => int)) {
    hh_expect_equivalent<nothing>($x);
  } else {
    hh_expect_equivalent<shape('a' => int, 'b' => string)>($x);
  }
}
